#pragma warning(disable:4100)
#pragma warning(disable:4996)

#include "Socket.h"

PKSOCKET COMMAND_NewSocket = NULL;
KMUTEX always_receive_MUTEX = { 0 };

WSK_REGISTRATION     WskRegistration = { 0, };
WSK_PROVIDER_NPI     WskProvider = { 0, };
WSK_CLIENT_DISPATCH  WskDispatch = { MAKE_WSK_VERSION(1,0), 0, NULL };

NTSTATUS KspAsyncContextCompletionRoutine(_In_ PDEVICE_OBJECT	DeviceObject, _In_ PIRP Irp, _In_ PKEVENT Completion_context) // 패킷이 정상적으로 송수신되었을 때
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);


    KeSetEvent(Completion_context, 0, FALSE); // 이벤트 해제 


    return STATUS_MORE_PROCESSING_REQUIRED;
}



/*TCP 송-수신*/
NTSTATUS Send_or_Receive_TCP_Server(PVOID Buffer, SIZE_T Buffer_size, SOCKET_INFORMATION send_or_receive, PKSOCKET* NewSocket_parm) {


    NTSTATUS status = STATUS_SUCCESS;

    //패킷 데이터 전송
    //char sample[] = "IM KERNEL_DRriver!@"; // 전송할 데이터
    //SIZE_T buffer_size = sizeof(sample); // 전송할 데이터 크기

    //PVOID buffer = &sample; // 전송용 포인터
    //PULONG buffer_size = (ULONG)&buffer_size; // 전송용 버퍼 크기

    WSK_BUF WskBuffer;
    WskBuffer.Offset = 0;
    WskBuffer.Length = Buffer_size;
    WskBuffer.Mdl = IoAllocateMdl(Buffer, (ULONG)WskBuffer.Length, FALSE, FALSE, NULL);

    __try {
        MmProbeAndLockPages(WskBuffer.Mdl, KernelMode, IoWriteAccess);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_ACCESS_VIOLATION;
        IoFreeMdl(WskBuffer.Mdl);
        return status;
    }

    KeResetEvent(&(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent));

    IoReuseIrp(((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp, STATUS_UNSUCCESSFUL);

    IoSetCompletionRoutine( // IRP 성공 처리 루틴 등록 
        ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp,
        &KspAsyncContextCompletionRoutine,
        &(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent),
        TRUE,
        TRUE,
        TRUE // IoCancelIrp() 로 중간에 강제적으로 KspAsyncContextCompletionRoutine 루틴 함수 호출을 취소하는 "취소루틴"을 KspAsyncContextCompletionRoutine 으로함
    );


    if (send_or_receive == TCP_DATA_SEND) {
        //Send*
        status = ((PKSOCKET)(*NewSocket_parm))->WskConnectionDispatch->WskSend(
            ((PKSOCKET)(*NewSocket_parm))->WskSocket,        // Socket
            &WskBuffer,               // Buffer
            0,                    // Flags
            ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp  // Irp - 연결 상태 결과 저장
        );
    }
    else if (send_or_receive == TCP_DATA_RECEIVE) {
        status = ((PKSOCKET)(*NewSocket_parm))->WskConnectionDispatch->WskReceive(
            ((PKSOCKET)(*NewSocket_parm))->WskSocket,        // Socket
            &WskBuffer,               // Buffer
            0,                    // Flags
            ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp  // Irp - 연결 상태 결과 저장
        );
    }
    else {
        IoFreeMdl(WskBuffer.Mdl);
        return STATUS_UNSUCCESSFUL;
    }


    //이벤트 대기 해야하는지 확인
    if (status == STATUS_PENDING) {


        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "STATUS_PENDING ... \n");


        status = KeWaitForSingleObject(
            &(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent),
            Executive,
            KernelMode,
            FALSE,
            NULL
        );

        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Send_TO_TCP_Server ,,  KeWaitForSingleObject 이벤트 해제 됨 \n");

        status = ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp->IoStatus.Status;
    }
    IoFreeMdl(WskBuffer.Mdl);
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "송수신 결과 --> %p\n", status);
    return status;
}


/*TCP 연결 초기화*/
NTSTATUS Make_TCP_Connection(_In_ PCHAR Server_ip, _In_ USHORT Server_port, PKSOCKET* NewSocket_parm) { // 초기화 된 소켓을 포인터로 받아서 수정 작업을 함. 

    WSK_CLIENT_NPI WskClientNpi;
    WskClientNpi.ClientContext = NULL;
    WskClientNpi.Dispatch = &WskDispatch;

    if (!NT_SUCCESS(WskRegister(&WskClientNpi, &WskRegistration))) {
        return STATUS_UNSUCCESSFUL;
    }
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "WskRegister 성공");

    if (!NT_SUCCESS(WskCaptureProviderNPI(&WskRegistration, WSK_INFINITE_WAIT, &WskProvider))) {
        return STATUS_UNSUCCESSFUL;
    }
    /*클라이언트 소켓 제작*/
    *NewSocket_parm = ExAllocatePoolWithTag(NonPagedPool, sizeof(KSOCKET), 'ABCD');
    if (!*NewSocket_parm) {
        return STATUS_UNSUCCESSFUL;
    }
    /*초기화*/
    KeInitializeEvent(&(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent), SynchronizationEvent, FALSE); // 이벤트 초기화

    KeInitializeMutex(&(((PKSOCKET)(*NewSocket_parm))->Mutex), 0);

    ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp = IoAllocateIrp(1, FALSE); // IRP초기화 
    if (((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp == NULL) {
        ExFreePoolWithTag(((PKSOCKET)(*NewSocket_parm)), 'ABCD');
        return STATUS_UNSUCCESSFUL;
    }

    IoSetCompletionRoutine( // IRP 콜백루틴 적용
        ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp,
        &KspAsyncContextCompletionRoutine,
        &(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent),
        TRUE,
        TRUE,
        TRUE
    );
    /*본격 소켓 제작*/
    NTSTATUS status = WskProvider.Dispatch->WskSocket(
        WskProvider.Client,
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP,
        WSK_FLAG_CONNECTION_SOCKET,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp
    );
    /*이벤트 대기확인*/
    if (status == STATUS_PENDING) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "STATUS_PENDING ...");
        KeWaitForSingleObject(
            &(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent),
            Executive,
            KernelMode,
            FALSE,
            NULL
        );

        status = ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp->IoStatus.Status;
    }
    /*소켓 제작 마무리*/
    if (NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "소켓제작성공!");
        ((PKSOCKET)(*NewSocket_parm))->WskSocket = (PWSK_SOCKET)((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp->IoStatus.Information;
        ((PKSOCKET)(*NewSocket_parm))->WskDispatch = (PVOID)((PKSOCKET)(*NewSocket_parm))->WskSocket->Dispatch;

    }

    /*Connect*/
    KeResetEvent(&(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent));

    //
    // Reuse the IRP.
    //

    IoReuseIrp(((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp, STATUS_UNSUCCESSFUL);

    IoSetCompletionRoutine(
        ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp,
        &KspAsyncContextCompletionRoutine,
        &(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent),
        TRUE,
        TRUE,
        TRUE
    );

    /*TCP 보내기 전 세션 중 서버로부터 받기 준비 세팅 */
    SOCKADDR_IN LocalAddress;
    LocalAddress.sin_family = AF_INET;
    LocalAddress.sin_addr.s_addr = INADDR_ANY;
    LocalAddress.sin_port = 0;

    status = ((PKSOCKET)(*NewSocket_parm))->WskConnectionDispatch->WskBind(
        ((PKSOCKET)(*NewSocket_parm))->WskSocket,          // Socket
        (PSOCKADDR)&LocalAddress,   // LocalAddress
        0,                          // Flags (reserved)
        ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp    // Irp
    );
    /*이벤트 대기 해야하는지 확인*/
    if (status == STATUS_PENDING) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "STATUS_PENDING ...");
        KeWaitForSingleObject(
            &(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent),
            Executive,
            KernelMode,
            FALSE,
            NULL
        );

        status = ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp->IoStatus.Status;
    }
    /*소켓 제작 마무리*/
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "바인딩 실패!");
        return STATUS_UNSUCCESSFUL;
    }

    /*Connect*/
    KeResetEvent(&(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent));

    //
    // Reuse the IRP.
    //

    IoReuseIrp(((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp, STATUS_UNSUCCESSFUL);

    IoSetCompletionRoutine(
        ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp,
        &KspAsyncContextCompletionRoutine,
        &(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent),
        TRUE,
        TRUE,
        TRUE
    );

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "바인딩 성공!");

    SOCKADDR_IN RemoteAddress;

    //RemoteAddress.sin_addr.S_un.S_addr = RtlUlongByteSwap(0xC0A80064); // 192 | 168 | 0 | 100 을 나타낸 것.(순서를 역순 취하도록 해주는 매크로)

    PCHAR ip_str = ExAllocatePoolWithTag(NonPagedPool, strlen(Server_ip) + 1, 'zxcv');
    strcpy_s(ip_str, strlen(Server_ip) + 1, Server_ip);
    PCHAR Next_str = NULL;
    PCHAR str_token = strtok_s(ip_str, ".", &Next_str);
    ULONG ip_byte[4] = { 0 };

    int i = 0;
    while (str_token != NULL && i < 4) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n 문자 -> %s \n", str_token);

        RtlCharToInteger(str_token, 10, &ip_byte[i]);
        i++;

        str_token = strtok_s(NULL, ".", &Next_str); // 다음 필드 추출

    }

    RemoteAddress.sin_addr.S_un.S_un_b.s_b1 = (UCHAR)ip_byte[0]; //A
    RemoteAddress.sin_addr.S_un.S_un_b.s_b2 = (UCHAR)ip_byte[1]; //B
    RemoteAddress.sin_addr.S_un.S_un_b.s_b3 = (UCHAR)ip_byte[2]; //C
    RemoteAddress.sin_addr.S_un.S_un_b.s_b4 = (UCHAR)ip_byte[3]; //D

    RemoteAddress.sin_family = AF_INET; // IPv4
    RemoteAddress.sin_port = RtlUshortByteSwap(Server_port); // Port 번호ㅇ

    status = ((PKSOCKET)(*NewSocket_parm))->WskConnectionDispatch->WskConnect(
        ((PKSOCKET)(*NewSocket_parm))->WskSocket,          // Socket
        (PSOCKADDR)&RemoteAddress,     // RemoteAddress
        0,                             // Flags (reserved)
        ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp    // Irp
    );

    ExFreePoolWithTag(ip_str, 'zxcv');
    /*이벤트 대기 해야하는지 확인*/
    if (status == STATUS_PENDING) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "STATUS_PENDING ...");
        KeWaitForSingleObject(
            &(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent),
            Executive,
            KernelMode,
            FALSE,
            NULL
        );
        status = ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp->IoStatus.Status;
    }
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "STATUS 결과 -> %p NewSocket 주소 -> %p  \n\n", status, ((PKSOCKET)(*NewSocket_parm)));


    KeInitializeMutex(&always_receive_MUTEX, 0); // 이벤트 인터럽트 시 지속적인 리시브 상태를 해제되는 충돌을 막기 위한 MUTEX 




    if (status != STATUS_SUCCESS) {
        ExFreePoolWithTag(((PKSOCKET)(*NewSocket_parm)), 'ABCD');
        ((PKSOCKET)(*NewSocket_parm)) = NULL;
        return STATUS_UNSUCCESSFUL;
    }
    return status;

}

///

VOID TCP_socket_release(PKSOCKET* NewSocket_parm) {
    if (!NewSocket_parm)
        return;

    if (!*NewSocket_parm)
        return;

    ExFreePoolWithTag(*NewSocket_parm, 'ABCD');
    *NewSocket_parm = NULL;
    return;
}