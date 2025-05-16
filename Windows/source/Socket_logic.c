#pragma warning(disable:4100)
#pragma warning(disable:4996)

#include "Socket.h"

PKSOCKET COMMAND_NewSocket = NULL;
KMUTEX always_receive_MUTEX = { 0 };

WSK_REGISTRATION     WskRegistration = { 0, };
WSK_PROVIDER_NPI     WskProvider = { 0, };
WSK_CLIENT_DISPATCH  WskDispatch = { MAKE_WSK_VERSION(1,0), 0, NULL };

NTSTATUS KspAsyncContextCompletionRoutine(_In_ PDEVICE_OBJECT	DeviceObject, _In_ PIRP Irp, _In_ PKEVENT Completion_context) // ��Ŷ�� ���������� �ۼ��ŵǾ��� ��
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);


    KeSetEvent(Completion_context, 0, FALSE); // �̺�Ʈ ���� 


    return STATUS_MORE_PROCESSING_REQUIRED;
}



/*TCP ��-����*/
NTSTATUS Send_or_Receive_TCP_Server(PVOID Buffer, SIZE_T Buffer_size, SOCKET_INFORMATION send_or_receive, PKSOCKET* NewSocket_parm) {


    NTSTATUS status = STATUS_SUCCESS;

    //��Ŷ ������ ����
    //char sample[] = "IM KERNEL_DRriver!@"; // ������ ������
    //SIZE_T buffer_size = sizeof(sample); // ������ ������ ũ��

    //PVOID buffer = &sample; // ���ۿ� ������
    //PULONG buffer_size = (ULONG)&buffer_size; // ���ۿ� ���� ũ��

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

    IoSetCompletionRoutine( // IRP ���� ó�� ��ƾ ��� 
        ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp,
        &KspAsyncContextCompletionRoutine,
        &(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent),
        TRUE,
        TRUE,
        TRUE // IoCancelIrp() �� �߰��� ���������� KspAsyncContextCompletionRoutine ��ƾ �Լ� ȣ���� ����ϴ� "��ҷ�ƾ"�� KspAsyncContextCompletionRoutine ������
    );


    if (send_or_receive == TCP_DATA_SEND) {
        //Send*
        status = ((PKSOCKET)(*NewSocket_parm))->WskConnectionDispatch->WskSend(
            ((PKSOCKET)(*NewSocket_parm))->WskSocket,        // Socket
            &WskBuffer,               // Buffer
            0,                    // Flags
            ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp  // Irp - ���� ���� ��� ����
        );
    }
    else if (send_or_receive == TCP_DATA_RECEIVE) {
        status = ((PKSOCKET)(*NewSocket_parm))->WskConnectionDispatch->WskReceive(
            ((PKSOCKET)(*NewSocket_parm))->WskSocket,        // Socket
            &WskBuffer,               // Buffer
            0,                    // Flags
            ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp  // Irp - ���� ���� ��� ����
        );
    }
    else {
        IoFreeMdl(WskBuffer.Mdl);
        return STATUS_UNSUCCESSFUL;
    }


    //�̺�Ʈ ��� �ؾ��ϴ��� Ȯ��
    if (status == STATUS_PENDING) {


        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "STATUS_PENDING ... \n");


        status = KeWaitForSingleObject(
            &(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent),
            Executive,
            KernelMode,
            FALSE,
            NULL
        );

        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Send_TO_TCP_Server ,,  KeWaitForSingleObject �̺�Ʈ ���� �� \n");

        status = ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp->IoStatus.Status;
    }
    IoFreeMdl(WskBuffer.Mdl);
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�ۼ��� ��� --> %p\n", status);
    return status;
}


/*TCP ���� �ʱ�ȭ*/
NTSTATUS Make_TCP_Connection(_In_ PCHAR Server_ip, _In_ USHORT Server_port, PKSOCKET* NewSocket_parm) { // �ʱ�ȭ �� ������ �����ͷ� �޾Ƽ� ���� �۾��� ��. 

    WSK_CLIENT_NPI WskClientNpi;
    WskClientNpi.ClientContext = NULL;
    WskClientNpi.Dispatch = &WskDispatch;

    if (!NT_SUCCESS(WskRegister(&WskClientNpi, &WskRegistration))) {
        return STATUS_UNSUCCESSFUL;
    }
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "WskRegister ����");

    if (!NT_SUCCESS(WskCaptureProviderNPI(&WskRegistration, WSK_INFINITE_WAIT, &WskProvider))) {
        return STATUS_UNSUCCESSFUL;
    }
    /*Ŭ���̾�Ʈ ���� ����*/
    *NewSocket_parm = ExAllocatePoolWithTag(NonPagedPool, sizeof(KSOCKET), 'ABCD');
    if (!*NewSocket_parm) {
        return STATUS_UNSUCCESSFUL;
    }
    /*�ʱ�ȭ*/
    KeInitializeEvent(&(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent), SynchronizationEvent, FALSE); // �̺�Ʈ �ʱ�ȭ

    KeInitializeMutex(&(((PKSOCKET)(*NewSocket_parm))->Mutex), 0);

    ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp = IoAllocateIrp(1, FALSE); // IRP�ʱ�ȭ 
    if (((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp == NULL) {
        ExFreePoolWithTag(((PKSOCKET)(*NewSocket_parm)), 'ABCD');
        return STATUS_UNSUCCESSFUL;
    }

    IoSetCompletionRoutine( // IRP �ݹ��ƾ ����
        ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp,
        &KspAsyncContextCompletionRoutine,
        &(((PKSOCKET)(*NewSocket_parm))->AsyncContext.CompletionEvent),
        TRUE,
        TRUE,
        TRUE
    );
    /*���� ���� ����*/
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
    /*�̺�Ʈ ���Ȯ��*/
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
    /*���� ���� ������*/
    if (NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�������ۼ���!");
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

    /*TCP ������ �� ���� �� �����κ��� �ޱ� �غ� ���� */
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
    /*�̺�Ʈ ��� �ؾ��ϴ��� Ȯ��*/
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
    /*���� ���� ������*/
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���ε� ����!");
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

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���ε� ����!");

    SOCKADDR_IN RemoteAddress;

    //RemoteAddress.sin_addr.S_un.S_addr = RtlUlongByteSwap(0xC0A80064); // 192 | 168 | 0 | 100 �� ��Ÿ�� ��.(������ ���� ���ϵ��� ���ִ� ��ũ��)

    PCHAR ip_str = ExAllocatePoolWithTag(NonPagedPool, strlen(Server_ip) + 1, 'zxcv');
    strcpy_s(ip_str, strlen(Server_ip) + 1, Server_ip);
    PCHAR Next_str = NULL;
    PCHAR str_token = strtok_s(ip_str, ".", &Next_str);
    ULONG ip_byte[4] = { 0 };

    int i = 0;
    while (str_token != NULL && i < 4) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n ���� -> %s \n", str_token);

        RtlCharToInteger(str_token, 10, &ip_byte[i]);
        i++;

        str_token = strtok_s(NULL, ".", &Next_str); // ���� �ʵ� ����

    }

    RemoteAddress.sin_addr.S_un.S_un_b.s_b1 = (UCHAR)ip_byte[0]; //A
    RemoteAddress.sin_addr.S_un.S_un_b.s_b2 = (UCHAR)ip_byte[1]; //B
    RemoteAddress.sin_addr.S_un.S_un_b.s_b3 = (UCHAR)ip_byte[2]; //C
    RemoteAddress.sin_addr.S_un.S_un_b.s_b4 = (UCHAR)ip_byte[3]; //D

    RemoteAddress.sin_family = AF_INET; // IPv4
    RemoteAddress.sin_port = RtlUshortByteSwap(Server_port); // Port ��ȣ��

    status = ((PKSOCKET)(*NewSocket_parm))->WskConnectionDispatch->WskConnect(
        ((PKSOCKET)(*NewSocket_parm))->WskSocket,          // Socket
        (PSOCKADDR)&RemoteAddress,     // RemoteAddress
        0,                             // Flags (reserved)
        ((PKSOCKET)(*NewSocket_parm))->AsyncContext.Irp    // Irp
    );

    ExFreePoolWithTag(ip_str, 'zxcv');
    /*�̺�Ʈ ��� �ؾ��ϴ��� Ȯ��*/
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
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "STATUS ��� -> %p NewSocket �ּ� -> %p  \n\n", status, ((PKSOCKET)(*NewSocket_parm)));


    KeInitializeMutex(&always_receive_MUTEX, 0); // �̺�Ʈ ���ͷ�Ʈ �� �������� ���ú� ���¸� �����Ǵ� �浹�� ���� ���� MUTEX 




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