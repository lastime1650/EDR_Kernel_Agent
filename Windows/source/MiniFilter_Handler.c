/*
    누수 발생 여부: 경고
*/
#include "MiniFilter_Handler.h"

K_EVENT_or_MUTEX_struct MiniFilter_mutex = {NULL, K_MUTEX , FALSE};

PShare_filter_Obj share_filter_object = NULL;

#include "is_system_pid.h" 

#include "DynamicData_2_lengthBuffer.h"
#include "DynamicData_Linked_list.h"
#include "Analysis_enum.h"
#include "Get_Time.h"
#include "converter_string.h"

BOOLEAN Is_File_with_Get_File_Info(
    PFLT_CALLBACK_DATA Input_Data, // 핸들러에서 얻은 정보
    PFLT_FILE_NAME_INFORMATION* Output_fileNameInfo // 정보 반환 (아니면 그대로 냅둔 ) 
);

BOOLEAN Init_return__edr_data(
    HANDLE* input__processid,
    PFLT_FILE_NAME_INFORMATION input_file_info,

    File_Behavior file_behavior,
    ULONG32 file_size,

    BOOLEAN is_DENIED,

    PDynamicData* output_start_address_node,
    PDynamicData* output_current_address_node
);


// 파일 사이즈 가져오기
BOOLEAN Flt_Get_File_Size(
    PFLT_CALLBACK_DATA Data,
    ULONG32* output_file_size
);

// 바이너리 파일 읽기 ( PASSIVE_LEVEL일 떄 ) 
BOOLEAN Flt_Get_File_Binary(
    PFLT_CALLBACK_DATA Data,

    PUCHAR* Buffer,
    ULONG32 input_BufferSize,

    PCHAR opt_inout_SHA256
);
// 바이너리 파일 읽기 ( 해제 ) 
VOID Flt_Get_File_Binary__Release(PUCHAR input_allocated_Buffer);


/*
=========
*/

#include "WorkItem_job.h"
#include "Response_File.h"
#include "File_io.h"

// Post-Operation에 전달할 컨텍스트 구조체 정의 (예시)
typedef struct _PRE_2_POST_CONTEXT {
    BOOLEAN IsDeleteOperation;
    // 필요하다면 파일 이름 등의 추가 정보 저장
    // PUNICODE_STRING FileName;
} PRE_2_POST_CONTEXT, * PPRE_2_POST_CONTEXT;

// PRE fix 핸들러
FLT_PREOP_CALLBACK_STATUS
PRE_filter_Handler(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    PCHAR Timestamp = Get_Time();

    if (KeGetCurrentIrql() != PASSIVE_LEVEL) {
        Release_Got_Time(Timestamp);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    // 시스템관련 프로세스인지 확인
    PEPROCESS Process_info = IoThreadToProcess(Data->Thread);
    HANDLE ProcessId = PsGetProcessId(Process_info);

    if (ProcessId < (HANDLE)100) {
        Release_Got_Time(Timestamp);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }
        

    if (Is_it_System_Process(ProcessId)){
        Release_Got_Time(Timestamp);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    // 파일인지 확인
    PFLT_FILE_NAME_INFORMATION fileNameInfo = NULL;
    if (Is_File_with_Get_File_Info(Data, &fileNameInfo) == FALSE) {
        Release_Got_Time(Timestamp);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    BOOLEAN is_Denied_RETURN = FALSE; // 행위 차단 여부

    ULONG32 file_size = 0;

    // 3. ZwCreateFile을 사용하여 파일 열기
    OBJECT_ATTRIBUTES objAttr;
    IO_STATUS_BLOCK ioStatusBlock;

    InitializeObjectAttributes(
        &objAttr,
        &fileNameInfo->Name,
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );
    HANDLE fileHandle = 0;
    NTSTATUS status = ZwCreateFile(
        &fileHandle,
        FILE_GENERIC_READ | SYNCHRONIZE, // 최소한의 권한만 요청
        &objAttr,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ, // 공유 허용
        FILE_OPEN, // 이미 존재하는 파일만 열기
        FILE_SYNCHRONOUS_IO_NONALERT, // 동기 I/O
        NULL,
        0
    );
    if (status != STATUS_SUCCESS) {
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[PRE-ZwCreateFile] 실패\n");
        FltReleaseFileNameInformation(fileNameInfo);
        Release_Got_Time(Timestamp);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    FILE_STANDARD_INFORMATION fileInfo;
    status = ZwQueryInformationFile(
        fileHandle,
        &ioStatusBlock,
        &fileInfo,
        sizeof(FILE_STANDARD_INFORMATION),
        FileStandardInformation
    );

    if (NT_SUCCESS(status)) {
        file_size = (ULONG32)fileInfo.EndOfFile.QuadPart;
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "파일 길이 구했는디? 경로: %wZ // 길이: %lu \n", fileNameInfo->Name, file_size);
    }

    ZwClose(fileHandle);
    
    // [ TEST ] 길이가 0 인 경우는 취급안함
    //if (file_size == 0)
        //goto NOTHING_return;
    /*
    status =  FILE_to_INFO(
        &fileNameInfo->Name,
        NULL,
        NULL,
        NULL,
        SHA256
    );
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[FILE_to_INFO] -> %wZ / %64s \n", fileNameInfo->Name, SHA256);
    */

        

    /*
        서버에 전달하기 위한 노드 준비
    */
    PDynamicData Start_Addr = NULL;
    PDynamicData Current_Addr = NULL;


    // IRP 별 처리
    switch (Data->Iopb->MajorFunction) {
    case IRP_MJ_CREATE:
    {

        // 차단 시도
        if (is_same_check(&fileNameInfo->Name, file_size)) { // 차단 여부 결정 
            /*
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "flt 차단 \n");
            Data->IoStatus.Status = STATUS_ACCESS_DENIED;
            Data->IoStatus.Information = 0;
            FltReleaseFileNameInformation(fileNameInfo);
            return FLT_PREOP_COMPLETE; // Block
            */
            goto DENIED_return;
        }




        //공유데이터 초기화
        memset(share_filter_object, 0, sizeof(Share_filter_Obj) - sizeof(KSPIN_LOCK));


        

        // Create ( 파일 생성 확인 )
        if (Data->Iopb->Parameters.Create.Options & FILE_CREATE) {
            // 파일 생성

            if (Init_return__edr_data(&ProcessId, fileNameInfo, Create, file_size, is_Denied_RETURN , &Start_Addr, &Current_Addr) == TRUE) { // 이것은 한번더 테스트해야햄 
               // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Pre-CREATE]미니필터 파일경로: %wZ / SecContext: %d / Options: %d \n", fileNameInfo->Name, Data->Iopb->Parameters.Create.SecurityContext, Data->Iopb->Parameters.Create.Options);
                goto To_Server;

            }
        }
        else {
            // 파일 열기

            // 쓰기 권한 확인
            if (Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & (FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES | GENERIC_WRITE)) {
                // 파일 쓰기 모드로 열기
               // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Pre-WRITE]미니필터 파일경로: %wZ / SecContext: %d / Options: %d \n", fileNameInfo->Name, Data->Iopb->Parameters.Create.SecurityContext, Data->Iopb->Parameters.Create.Options);
                KIRQL oldIrql;
                KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
                share_filter_object->IsWriteMode = TRUE;
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "미니필터 [PID]: %llu 파일 쓰기 모드로 열기 \n", ProcessId);
            }

            // 읽기 권한 확인 (쓰기 권한과 별도로)
            if (Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & (FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA | GENERIC_READ)) {
               // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Pre-READ]미니필터 파일경로: %wZ / SecContext: %d / Options: %d \n", fileNameInfo->Name, Data->Iopb->Parameters.Create.SecurityContext, Data->Iopb->Parameters.Create.Options);
                // 파일 읽기 모드로 열기
                KIRQL oldIrql;
                KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
                share_filter_object->IsReadMode = TRUE;
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "미니필터 [PID]: %llu 파일 읽기 모드로 열기 \n", ProcessId);
            }
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "미니필터 [PID]: %llu 쓰기:%d / 읽기:%d \n", ProcessId, share_filter_object->IsWriteMode, share_filter_object->IsReadMode);

            // 삭제 권한 확인
            if (Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & DELETE) {
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Pre-DELETE]미니필터 파일경로: %wZ / SecContext: %d / Options: %d \n", fileNameInfo->Name, Data->Iopb->Parameters.Create.SecurityContext, Data->Iopb->Parameters.Create.Options);
            }
        }



        break;
    }



    case IRP_MJ_WRITE:
    {
        // 파일 쓰기
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "미니필터 [PID]: %llu 파일 쓰기 \n", ProcessId);
        KIRQL oldIrql;
        KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
        if (share_filter_object->IsWriteMode) {
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "미니필터 [PID]: %llu 파일 쓰기 모드 확인 \n", ProcessId);

            if (Init_return__edr_data(&ProcessId, fileNameInfo, Write, file_size, is_Denied_RETURN, &Start_Addr, &Current_Addr) == TRUE) {
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[WRITE]미니필터 파일경로: %wZ  \n", fileNameInfo->Name);
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                goto To_Server;

            }
        }
        KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);

        break;
    }
    case IRP_MJ_READ:
    {

        // 파일 읽기
        KIRQL oldIrql;
        KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
        if (share_filter_object->IsReadMode) {
            

            if (Init_return__edr_data(&ProcessId, fileNameInfo, Read, file_size, is_Denied_RETURN, &Start_Addr, &Current_Addr) == TRUE) {
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[READ]미니필터 파일경로: %wZ  \n", fileNameInfo->Name);
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                goto To_Server;

            }

            
        }
        KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
        break;

    }
    case IRP_MJ_SET_INFORMATION:
    {
        // 1. 파일 이름 변경 필터링
        if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileRenameInformation ||
            Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileRenameInformationEx)
        {

            //UNICODE_STRING newFileName = { 0, };
            PFILE_RENAME_INFORMATION renameInfo = (PFILE_RENAME_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
            if (renameInfo != NULL)
            {

                //RtlInitUnicodeString(&newFileName, renameInfo->FileName);

                // EDR server에넘길것
                if (Init_return__edr_data(&ProcessId, fileNameInfo, Rename, file_size, is_Denied_RETURN, &Start_Addr, &Current_Addr) == TRUE) {
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Rename]미니필터 파일경로: %wZ  \n", fileNameInfo->Name);
                    // 변경되는 이름 추가 ( PWCH )
                    Current_Addr = AppendDynamicData(Current_Addr, (PUCHAR)renameInfo->FileName, renameInfo->FileNameLength);

                    // 

                    goto To_Server;
                }



            }
        }
        // 2. 파일 삭제 확인
        else if (
            Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformation ||
            Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformationEx
            )
        {
            PFILE_DISPOSITION_INFORMATION delnameInfo = (PFILE_DISPOSITION_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
            if (delnameInfo->DeleteFile) {
                // EDR server에넘길것
                if (Init_return__edr_data(&ProcessId, fileNameInfo, Delete, file_size, is_Denied_RETURN, &Start_Addr, &Current_Addr) == TRUE) {
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[DELETE]미니필터 파일경로: %wZ  \n", fileNameInfo->Name);
                    // 따로 뭐 없음. 현재 'fileNameInfo'에 표시하는 현재 파일명이 삭제 될 것임 
                    //is_Denied_RETURN = TRUE; // TRUE면 차단
                    goto To_Server;
                }
            }
        }


        break;
    }
    default:
        break;
    }

    goto _return;

To_Server:
    {
        // CMD 추출
        Analysis_Command cmd = File_System;
        // PID 추출
        HANDLE pid = ProcessId;

        // 동적 데이터 추출
        PDynamicData start_node = Start_Addr;
        /*
            전달용 동적데이터
        */
        Pmover move = ExAllocatePoolWithTag(NonPagedPool, sizeof(mover), mover_tag);
        if (!move) {
            RemoveALLDynamicData(Start_Addr);
            Release_Got_Time(Timestamp);
            goto _return;
        }

        move->cmd = cmd;
        move->PID = pid;
        move->start_node = start_node;
        move->timestamp = Timestamp;


        // WORK

        PWORK_ITEM work_info = ExAllocatePoolWithTag(NonPagedPool, sizeof(WORK_ITEM), WORK_ITEM_TAG);
        if (!work_info) {
            RemoveALLDynamicData(move->start_node);
            Release_Got_Time(Timestamp);
            ExFreePoolWithTag(move, mover_tag);
            goto _return;
        }
        work_info->context.startroutine = Dyn_2_lenBuff;
        work_info->context.context = move;
        ExInitializeWorkItem(&work_info->reserved, WORK_job, work_info);
        ExQueueWorkItem(&work_info->reserved, NormalWorkQueue);

        goto _return;
    }

_return:
{
    if (!is_Denied_RETURN) {
        goto NOTHING_return;
    }
    else {
        goto DENIED_return;
    }
}

NOTHING_return:
    {
        // 정상 리턴
        FltReleaseFileNameInformation(fileNameInfo);
        //return FLT_PREOP_SUCCESS_NO_CALLBACK;
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

DENIED_return:
    {
        // 행위 차단 리턴
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        Data->IoStatus.Information = 0;
        FltReleaseFileNameInformation(fileNameInfo);
        return FLT_PREOP_COMPLETE; // Block

    }
}





BOOLEAN Init_return__edr_data(
    HANDLE* input__processid,
    PFLT_FILE_NAME_INFORMATION input_file_info,

    File_Behavior file_behavior,
    
    ULONG32 file_size,

    BOOLEAN is_DENIED,

    PDynamicData* output_start_address_node,
    PDynamicData* output_current_address_node
) {
    if (input__processid == NULL || input_file_info == NULL || output_start_address_node == NULL || output_current_address_node == NULL)
        return FALSE;


    // [ EDR-Server ] File_Name ( IRQL 문제로 문자열 변환 안함 ) 
    *output_start_address_node = CreateDynamicData((PUCHAR)input_file_info->Name.Buffer, input_file_info->Name.MaximumLength);
    *output_current_address_node = *output_start_address_node;

    // 파일 길이
    *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&file_size, sizeof(file_size));

    // + 차단 이력도 확인한다
    if (is_DENIED) {
        UCHAR DENIED[] = "DENIED";
        *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&DENIED, sizeof(DENIED) - 1);
    }
    else {
        UCHAR GRANTED[] = "GRANTED";
        *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&GRANTED, sizeof(GRANTED) - 1);
    }



    // [EDR-Server] Behavior - str 파일 행동
    switch (file_behavior) {
    case Create:
    {
        UCHAR behavior[] = "Create";
        *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&behavior, sizeof(behavior) - 1);
        break;
    }
    case Write:
    {
        UCHAR behavior[] = "Write";
        *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&behavior, sizeof(behavior) - 1);
        break;
    }
    case Read:
    {
        UCHAR behavior[] = "Read";
        *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&behavior, sizeof(behavior) - 1);
        break;
    }
    case Rename:
    {
        UCHAR behavior[] = "Rename";
        *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&behavior, sizeof(behavior) - 1);
        break;
    }
    case Delete:
    {
        UCHAR behavior[] = "Delete";
        *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&behavior, sizeof(behavior) - 1);
        break;
    }
    default:
    {
        // Unknown
        UCHAR behavior[] = "Unknown";
        *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&behavior, sizeof(behavior) - 1);
        break;
    }
    }

    

    return TRUE;
}






///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//일반-파일인지 Check
BOOLEAN Is_File_with_Get_File_Info(
    PFLT_CALLBACK_DATA Input_Data, // 핸들러에서 얻은 정보
    PFLT_FILE_NAME_INFORMATION* Output_fileNameInfo // 정보 반환 (아니면 그대로 냅둔 ) 
) {
    if (Output_fileNameInfo == NULL) return FALSE;

    // 1차 관문 -- 유저모드에 의한 요청인지 체크
    if (Input_Data->RequestorMode != UserMode) return FALSE;

    // 2차 관문 -- 파일이라면 아래와 같은 API가 성공함 
    if (FltGetFileNameInformation(Input_Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, Output_fileNameInfo) != STATUS_SUCCESS)
        return FALSE;

    // 파일 이름 추출
    if (FltParseFileNameInformation(*Output_fileNameInfo) != STATUS_SUCCESS) {

        FltReleaseFileNameInformation(*Output_fileNameInfo);
        return FALSE;
    }

    return TRUE;
}


// 파일 사이즈 가져오기
BOOLEAN Flt_Get_File_Size(
    PFLT_CALLBACK_DATA Data,
    ULONG32* output_file_size
) {
    if (KeGetCurrentIrql() != PASSIVE_LEVEL )//&& KeGetCurrentIrql() != APC_LEVEL)
        return FALSE;

    FILE_STANDARD_INFORMATION fileInfo;
    NTSTATUS status = FltQueryInformationFile(
        Data->Iopb->TargetInstance,
        Data->Iopb->TargetFileObject,
        &fileInfo,
        sizeof(FILE_STANDARD_INFORMATION),
        FileStandardInformation,
        NULL
    );
    if(status != STATUS_SUCCESS) {
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Size 파일 크기 못구함 %p \n", status);
        return FALSE;
    }

    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Size 파일 크기 구함 -> %lu \n", fileInfo.EndOfFile.QuadPart);

    *output_file_size = (ULONG32)fileInfo.EndOfFile.QuadPart;
    return TRUE;
}

/*
// 바이너리 파일 읽기 ( PASSIVE_LEVEL일 떄 ) 
BOOLEAN Flt_Get_File_Binary(
    PFLT_CALLBACK_DATA Data,

    PUCHAR* Buffer,
    ULONG32 input_BufferSize,

    PCHAR opt_inout_SHA256
) {
    if (KeGetCurrentIrql() != PASSIVE_LEVEL || input_BufferSize > 500000000)
        return FALSE;

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Binary 호출됨 \n");
    *Buffer = ExAllocatePoolWithTag(PagedPool, input_BufferSize, 'FLTm');


    if (FltReadFile(
        Data->Iopb->TargetInstance,
        Data->Iopb->TargetFileObject,
        NULL, 
        input_BufferSize,
        *Buffer,
        FLTFL_IO_OPERATION_NON_CACHED | FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET,
        NULL,
        NULL,
        NULL
    ) != STATUS_SUCCESS)
        return FALSE;

    if (opt_inout_SHA256) {
        NTSTATUS status = GET_SHA256_HASHING(*Buffer, input_BufferSize, opt_inout_SHA256);
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Binary 해시구하기 -> %p \n", status);
    }

    return TRUE;

}

VOID Flt_Get_File_Binary__Release(PUCHAR input_allocated_Buffer) {
    if(input_allocated_Buffer)
        ExFreePoolWithTag(input_allocated_Buffer, 'FLTm');
}*/

/*
#define CHUNK_READ_SIZE 4096 // 한 번에 읽을 청크 크기 (예: 4KB)
#define CHUNK_POOL_TAG 'RtsP' // 메모리 할당 태그 (예: 'PostR'ead)



FLT_POSTOP_CALLBACK_STATUS
PostSetInfoCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext, // Pre-Op에서 전달된 컨텍스트
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{

    // 시스템관련 프로세스인지 확인
    PEPROCESS Process_info = IoThreadToProcess(Data->Thread);
    HANDLE ProcessId = PsGetProcessId(Process_info);

    if (ProcessId < (HANDLE)100)
        return FLT_POSTOP_FINISHED_PROCESSING;

    if (Is_it_System_Process(ProcessId))
        return FLT_POSTOP_FINISHED_PROCESSING;

    //

    NTSTATUS status;
    PVOID chunkBuffer = NULL;
    ULONG bytesActuallyRead = 0;
    LARGE_INTEGER readOffset;

    UNREFERENCED_PARAMETER(CompletionContext);

    // 1. Draining 상태 확인: 필터 언로드 중에는 동기 I/O 금지
    if (FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING)) {
        // 언로드 중이므로 추가 작업 없이 종료
        // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "PostReadCallback: Filter is draining, skipping chunk read.\n"); // 정보 레벨로 변경 가능
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    // 2. 원래 Read 작업 성공 여부 확인 (선택 사항이지만 권장)
    //    - 실패한 Read 후에 여기서 다시 읽는 것은 의미가 없거나 위험할 수 있음
    //    - NT_SUCCESS는 STATUS_END_OF_FILE도 포함할 수 있음에 유의
    if (!NT_SUCCESS(Data->IoStatus.Status) && Data->IoStatus.Status != STATUS_END_OF_FILE) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: Original read failed (0x%X), skipping chunk read.\n", Data->IoStatus.Status);
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    // 3. 유효한 객체 확인
    if (FltObjects == NULL || FltObjects->FileObject == NULL || FltObjects->Instance == NULL) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: Invalid FltObjects.\n");
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: Starting chunk read for FileObject %p\n", FltObjects->FileObject); // 정보 레벨로 변경 가능
    
    // 4. 청크 읽기를 위한 버퍼 할당
    chunkBuffer = ExAllocatePoolZero(POOL_FLAG_NON_PAGED, CHUNK_READ_SIZE, CHUNK_POOL_TAG);
    if (chunkBuffer == NULL) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: Failed to allocate chunk buffer.\n");
        // 버퍼 할당 실패 시에도 원래 작업 결과는 변경하지 않음
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    // 5. 파일 시작부터 청크 단위로 읽기 루프
    readOffset.QuadPart = 0; // 파일 시작 오프셋
    do {
        // FltReadFile 호출하여 청크 읽기
        status = FltReadFile(
            FltObjects->Instance,       // 필터 인스턴스
            FltObjects->FileObject,     // 대상 파일 객체
            &readOffset,                // 읽기 시작 위치 (파일 시작부터 증가)
            CHUNK_READ_SIZE,            // 읽을 바이트 수 (청크 크기)
            chunkBuffer,                // 데이터를 저장할 버퍼
            FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET, // FileObject의 현재 위치를 변경하지 않음
            &bytesActuallyRead,         // 실제로 읽은 바이트 수
            NULL,                       // 동기 호출이므로 콜백 없음
            NULL                        // 콜백 컨텍스트 없음
        );

        if (NT_SUCCESS(status)) {
            if (bytesActuallyRead > 0) {
                // 성공적으로 청크를 읽음
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: Read %u bytes at offset %lld\n", bytesActuallyRead, readOffset.QuadPart); // 정보 레벨로 변경 가능

                // ==================================================
                // 여기에 읽은 청크(chunkBuffer)에 대한 처리 로직 추가
                // 예: 내용 검사, 해시 계산, 로그 기록 등
                // ProcessChunkData(chunkBuffer, bytesActuallyRead);
                // ==================================================

                // 다음 읽기 위치로 오프셋 이동
                readOffset.QuadPart += bytesActuallyRead;
            }
            else {
                // 성공했지만 읽은 바이트가 0 -> 파일 끝(EOF) 도달
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: EOF reached during chunk read.\n"); // 정보 레벨로 변경 가능
                break; // 루프 종료
            }
        }
        else if (status == STATUS_END_OF_FILE) {
            // 명시적으로 EOF 상태 반환됨
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: FltReadFile returned STATUS_END_OF_FILE.\n"); // 정보 레벨로 변경 가능
            bytesActuallyRead = 0; // 루프 종료 조건 만족시키기 위해
            break; // 루프 종료
        }
        else {
            // 읽기 중 오류 발생
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: FltReadFile failed with status 0x%X at offset %lld\n", status, readOffset.QuadPart);
            // 여기서의 오류는 기록만 하고 루프 종료. 원래 I/O 상태는 변경하지 않음.
            break; // 루프 종료
        }

        // 읽은 바이트가 있고, 청크 크기만큼 읽었을 수 있으므로 계속 시도
    } while (bytesActuallyRead > 0); // 실제 읽은 바이트가 있을 때까지 반복

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: Finished chunk read.\n"); // 정보 레벨로 변경 가능

    // 6. 할당했던 버퍼 해제 (루프 종료 후 반드시 수행)
    if (chunkBuffer != NULL) {
        ExFreePoolWithTag(chunkBuffer, CHUNK_POOL_TAG);
    }

    // 7. Post-Operation 처리 완료 반환
    //    - 여기서 발생한 오류로 인해 원래 IRP의 상태를 변경하지 않음
    return FLT_POSTOP_FINISHED_PROCESSING;
}*/