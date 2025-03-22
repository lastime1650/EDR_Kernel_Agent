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


#include "WorkItem_job.h"
#include "Response_File.h"
#include "File_io.h"
/*FLT_PREOP_CALLBACK_STATUS
Pre_filter_Handler(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
) */


FLT_POSTOP_CALLBACK_STATUS POST_filter_Handler(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);

    // 시스템관련 프로세스인지 확인
    PEPROCESS Process_info = IoThreadToProcess(Data->Thread);
    HANDLE ProcessId = PsGetProcessId(Process_info);
    
    if (ProcessId < (HANDLE)100)
        //return FLT_PREOP_SUCCESS_NO_CALLBACK;
        return FLT_POSTOP_FINISHED_PROCESSING;

    if (Is_it_System_Process(ProcessId)) 
        //return FLT_PREOP_SUCCESS_NO_CALLBACK;
        return FLT_POSTOP_FINISHED_PROCESSING;


    // 파일인지 확인
    PFLT_FILE_NAME_INFORMATION fileNameInfo = NULL;
    if (Is_File_with_Get_File_Info(Data, &fileNameInfo) == FALSE) {
        //return FLT_PREOP_SUCCESS_NO_CALLBACK;
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    ULONG32 file_size = 0;
    Flt_Get_File_Size(Data, &file_size);
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "파일 크기 -> %lu \n", file_size);

    /*
        서버에 전달하기 위한 노드 준비
    */
    PDynamicData Start_Addr = NULL;
    PDynamicData Current_Addr = NULL;


    // IRP 별 처리
    switch (Data->Iopb->MajorFunction) {
    case IRP_MJ_CREATE:
    {

        

        /*
            [사전 BLOCK 처리]
        */
        if (File_Response_Start_Node_Address != NULL) {

            ULONG32 File_Size = 0;
            if (get_file_size(&fileNameInfo->Name,NULL, &File_Size, TRUE)&& File_Size != 0) {// 해당 파일의 사이즈 가져오기
                if (is_same_check(&fileNameInfo->Name, File_Size)) { // 차단 여부 결정 
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "flt 차단 \n");
                    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                    Data->IoStatus.Information = 0;
                    //return FLT_PREOP_COMPLETE; // Block
                    return FLT_POSTOP_FINISHED_PROCESSING;
                }
            }
           // }

        }
        






        //공유데이터 초기화
        memset(share_filter_object, 0, sizeof(Share_filter_Obj) - sizeof(KSPIN_LOCK));

        // Create ( 파일 생성 확인 )
        if (Data->Iopb->Parameters.Create.Options & FILE_CREATE) {
            // 파일 생성
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "미니필터 [PID]: %llu 파일 생성 %d \n", ProcessId, Data->Iopb->Parameters.Create.Options);
            if (Init_return__edr_data(&ProcessId, fileNameInfo, Create, file_size, &Start_Addr, &Current_Addr) == TRUE) { // 이것은 한번더 테스트해야햄 

                goto To_Server;

            }
        }
        else {
            // 파일 열기

            // 쓰기 권한 확인
            if (Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & (FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES | GENERIC_WRITE)) {
                // 파일 쓰기 모드로 열기
                KIRQL oldIrql;
                KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
                share_filter_object->IsWriteMode = TRUE;
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "미니필터 [PID]: %llu 파일 쓰기 모드로 열기 \n", ProcessId);
            }

            // 읽기 권한 확인 (쓰기 권한과 별도로)
            if (Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & (FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA | GENERIC_READ)) {
                // 파일 읽기 모드로 열기
                KIRQL oldIrql;
                KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
                share_filter_object->IsReadMode = TRUE;
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "미니필터 [PID]: %llu 파일 읽기 모드로 열기 \n", ProcessId);
            }
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "미니필터 [PID]: %llu 쓰기:%d / 읽기:%d \n", ProcessId, share_filter_object->IsWriteMode, share_filter_object->IsReadMode);
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

            if (Init_return__edr_data(&ProcessId, fileNameInfo, Write, file_size , &Start_Addr, &Current_Addr) == TRUE) {
                
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                goto To_Server;

            }
        }
        KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);

        break;
    }
    case IRP_MJ_READ:
    {
        break;
        /*
        // 파일 읽기
        KIRQL oldIrql;
        KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
        if (share_filter_object->IsReadMode) {
           // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "미니필터 [PID]: %llu 파일 읽기 모드 확인 \n", ProcessId);

            if (Init_return__edr_data(&ProcessId, fileNameInfo, Read, &Start_Addr, &Current_Addr) == TRUE) {

                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                goto To_Server;

            }

            KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
            goto To_Server;
        }
        KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
        break;
        */
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
                if (Init_return__edr_data(&ProcessId, fileNameInfo, Rename, file_size, &Start_Addr, &Current_Addr) == TRUE) {

                    // 변경되는 이름 추가 ( PWCH )
                    Current_Addr = AppendDynamicData(Current_Addr, (PUCHAR)renameInfo->FileName, renameInfo->FileNameLength);
                    
                    // 

                    goto To_Server;
                }
                

                
            }
            /*1. 파일명 감지는 어떻게 알지?*/
            /*2. 파일 이동시, 그 위치는 어떻게 알지? ( 파싱 필요 ) */
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " 파일 이름 변경 감지@@ 변경될 명 -> %wZ\n", newFileName);

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
                if (Init_return__edr_data(&ProcessId, fileNameInfo, Delete, file_size, &Start_Addr, &Current_Addr) == TRUE) {
                    // 따로 뭐 없음. 현재 'fileNameInfo'에 표시하는 현재 파일명이 삭제 될 것임 
                    goto To_Server;
                }
            }
        }
            
            
        break;
    }
    default:
        break;
    }
    
    goto NOTHING_return;

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
            goto NOTHING_return;
        }

        move->cmd = cmd;
        move->PID = pid;
        move->start_node = start_node;
        move->timestamp = Get_Time();

        
        // WORK

        PWORK_ITEM work_info = ExAllocatePoolWithTag(NonPagedPool, sizeof(WORK_ITEM), WORK_ITEM_TAG);
        if (!work_info) {
            RemoveALLDynamicData(move->start_node);
            Release_Got_Time(move->timestamp);
            ExFreePoolWithTag(move, mover_tag);
            goto NOTHING_return;
        }
        work_info->context.startroutine = Dyn_2_lenBuff;
        work_info->context.context = move;
        ExInitializeWorkItem(&work_info->reserved, WORK_job, work_info);
        ExQueueWorkItem(&work_info->reserved, NormalWorkQueue);
        
        goto NOTHING_return;
    }

NOTHING_return:
    {
        // 정상 리턴
        FltReleaseFileNameInformation(fileNameInfo);
        //return FLT_PREOP_SUCCESS_NO_CALLBACK;
        return FLT_POSTOP_FINISHED_PROCESSING;
    }
}



// PRE fix 핸들러
FLT_PREOP_CALLBACK_STATUS
PRE_filter_Handler(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    // 파일인지 확인
    PFLT_FILE_NAME_INFORMATION fileNameInfo = NULL;
    if (Is_File_with_Get_File_Info(Data, &fileNameInfo) == FALSE) {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    

        if (Data->Iopb->MajorFunction == IRP_MJ_CREATE)
        {
            if (KeGetCurrentIrql() == PASSIVE_LEVEL) {
                ULONG32 file_size = 0;
                Flt_Get_File_Size(Data, &file_size);



                /*
                    [BLOCK 처리]
                */

                // 파일길이 얻기
                if (file_size == 0) {
                    ULONG32 tmp_size = 0;
                    if (!get_file_size(&fileNameInfo->Name, NULL, &tmp_size, TRUE) || tmp_size == 0) {// 해당 파일의 사이즈 가져오기
                        return FLT_PREOP_SUCCESS_NO_CALLBACK;
                    }
                    file_size = tmp_size;
                }

                // 차단 시도
                if (is_same_check(&fileNameInfo->Name, file_size)) { // 차단 여부 결정 
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "flt 차단 \n");
                    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                    Data->IoStatus.Information = 0;
                    return FLT_PREOP_COMPLETE; // Block
                }


            }
            
        }

    

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}



/*
    누수 발생 여부: 양호
*/
BOOLEAN Init_return__edr_data(
    HANDLE* input__processid,
    PFLT_FILE_NAME_INFORMATION input_file_info,

    File_Behavior file_behavior,
    
    ULONG32 file_size,

    PDynamicData* output_start_address_node,
    PDynamicData* output_current_address_node
) {
    if (input__processid == NULL || input_file_info == NULL || output_start_address_node == NULL || output_current_address_node == NULL)
        return FALSE;

    /*
        PASSIVE_LEVEL이 아닌 경우가 있어,  BSOD 발생률이 높아, 변환작업은 이루어져서는 안된다. 
    */

    // [ EDR-Server ] File_Name ( IRQL 문제로 문자열 변환 안함 ) 
    *output_start_address_node = CreateDynamicData((PUCHAR)input_file_info->Name.Buffer, input_file_info->Name.MaximumLength);
    *output_current_address_node = *output_start_address_node;

    // 파일 길이
    *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&file_size, sizeof(file_size));

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
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Size 파일 크기 못구함 %p \n", status);
        return FALSE;
    }

    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Size 파일 크기 구함 -> %lu \n", fileInfo.EndOfFile.QuadPart);

    *output_file_size = (ULONG32)fileInfo.EndOfFile.QuadPart;
    return TRUE;
}


// 바이너리 파일 읽기 ( PASSIVE_LEVEL일 떄 ) 
#include "SHA256.h"
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
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Binary 해시구하기 \n");
        GET_SHA256_HASHING(*Buffer, input_BufferSize, opt_inout_SHA256);
    }

    return TRUE;

}

VOID Flt_Get_File_Binary__Release(PUCHAR input_allocated_Buffer) {
    if(input_allocated_Buffer)
        ExFreePoolWithTag(input_allocated_Buffer, 'FLTm');
}