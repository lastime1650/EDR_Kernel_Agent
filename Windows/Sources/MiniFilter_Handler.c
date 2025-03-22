/*
    ���� �߻� ����: ���
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
    PFLT_CALLBACK_DATA Input_Data, // �ڵ鷯���� ���� ����
    PFLT_FILE_NAME_INFORMATION* Output_fileNameInfo // ���� ��ȯ (�ƴϸ� �״�� ���� ) 
);

BOOLEAN Init_return__edr_data(
    HANDLE* input__processid,
    PFLT_FILE_NAME_INFORMATION input_file_info,

    File_Behavior file_behavior,
    ULONG32 file_size,

    PDynamicData* output_start_address_node,
    PDynamicData* output_current_address_node
);


// ���� ������ ��������
BOOLEAN Flt_Get_File_Size(
    PFLT_CALLBACK_DATA Data,
    ULONG32* output_file_size
);

// ���̳ʸ� ���� �б� ( PASSIVE_LEVEL�� �� ) 
BOOLEAN Flt_Get_File_Binary(
    PFLT_CALLBACK_DATA Data,

    PUCHAR* Buffer,
    ULONG32 input_BufferSize,

    PCHAR opt_inout_SHA256
);
// ���̳ʸ� ���� �б� ( ���� ) 
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

    // �ý��۰��� ���μ������� Ȯ��
    PEPROCESS Process_info = IoThreadToProcess(Data->Thread);
    HANDLE ProcessId = PsGetProcessId(Process_info);
    
    if (ProcessId < (HANDLE)100)
        //return FLT_PREOP_SUCCESS_NO_CALLBACK;
        return FLT_POSTOP_FINISHED_PROCESSING;

    if (Is_it_System_Process(ProcessId)) 
        //return FLT_PREOP_SUCCESS_NO_CALLBACK;
        return FLT_POSTOP_FINISHED_PROCESSING;


    // �������� Ȯ��
    PFLT_FILE_NAME_INFORMATION fileNameInfo = NULL;
    if (Is_File_with_Get_File_Info(Data, &fileNameInfo) == FALSE) {
        //return FLT_PREOP_SUCCESS_NO_CALLBACK;
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    ULONG32 file_size = 0;
    Flt_Get_File_Size(Data, &file_size);
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���� ũ�� -> %lu \n", file_size);

    /*
        ������ �����ϱ� ���� ��� �غ�
    */
    PDynamicData Start_Addr = NULL;
    PDynamicData Current_Addr = NULL;


    // IRP �� ó��
    switch (Data->Iopb->MajorFunction) {
    case IRP_MJ_CREATE:
    {

        

        /*
            [���� BLOCK ó��]
        */
        if (File_Response_Start_Node_Address != NULL) {

            ULONG32 File_Size = 0;
            if (get_file_size(&fileNameInfo->Name,NULL, &File_Size, TRUE)&& File_Size != 0) {// �ش� ������ ������ ��������
                if (is_same_check(&fileNameInfo->Name, File_Size)) { // ���� ���� ���� 
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "flt ���� \n");
                    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                    Data->IoStatus.Information = 0;
                    //return FLT_PREOP_COMPLETE; // Block
                    return FLT_POSTOP_FINISHED_PROCESSING;
                }
            }
           // }

        }
        






        //���������� �ʱ�ȭ
        memset(share_filter_object, 0, sizeof(Share_filter_Obj) - sizeof(KSPIN_LOCK));

        // Create ( ���� ���� Ȯ�� )
        if (Data->Iopb->Parameters.Create.Options & FILE_CREATE) {
            // ���� ����
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�̴����� [PID]: %llu ���� ���� %d \n", ProcessId, Data->Iopb->Parameters.Create.Options);
            if (Init_return__edr_data(&ProcessId, fileNameInfo, Create, file_size, &Start_Addr, &Current_Addr) == TRUE) { // �̰��� �ѹ��� �׽�Ʈ�ؾ��� 

                goto To_Server;

            }
        }
        else {
            // ���� ����

            // ���� ���� Ȯ��
            if (Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & (FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES | GENERIC_WRITE)) {
                // ���� ���� ���� ����
                KIRQL oldIrql;
                KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
                share_filter_object->IsWriteMode = TRUE;
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�̴����� [PID]: %llu ���� ���� ���� ���� \n", ProcessId);
            }

            // �б� ���� Ȯ�� (���� ���Ѱ� ������)
            if (Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & (FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA | GENERIC_READ)) {
                // ���� �б� ���� ����
                KIRQL oldIrql;
                KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
                share_filter_object->IsReadMode = TRUE;
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�̴����� [PID]: %llu ���� �б� ���� ���� \n", ProcessId);
            }
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�̴����� [PID]: %llu ����:%d / �б�:%d \n", ProcessId, share_filter_object->IsWriteMode, share_filter_object->IsReadMode);
        }

        break;
    }
        
        

    case IRP_MJ_WRITE:
    {
        // ���� ����
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�̴����� [PID]: %llu ���� ���� \n", ProcessId);
        KIRQL oldIrql;
        KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
        if (share_filter_object->IsWriteMode) {
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�̴����� [PID]: %llu ���� ���� ��� Ȯ�� \n", ProcessId);

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
        // ���� �б�
        KIRQL oldIrql;
        KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
        if (share_filter_object->IsReadMode) {
           // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�̴����� [PID]: %llu ���� �б� ��� Ȯ�� \n", ProcessId);

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
        // 1. ���� �̸� ���� ���͸�
        if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileRenameInformation ||
            Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileRenameInformationEx)
        {

            //UNICODE_STRING newFileName = { 0, };
            PFILE_RENAME_INFORMATION renameInfo = (PFILE_RENAME_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
            if (renameInfo != NULL)
            {

                //RtlInitUnicodeString(&newFileName, renameInfo->FileName);
                
                // EDR server���ѱ��
                if (Init_return__edr_data(&ProcessId, fileNameInfo, Rename, file_size, &Start_Addr, &Current_Addr) == TRUE) {

                    // ����Ǵ� �̸� �߰� ( PWCH )
                    Current_Addr = AppendDynamicData(Current_Addr, (PUCHAR)renameInfo->FileName, renameInfo->FileNameLength);
                    
                    // 

                    goto To_Server;
                }
                

                
            }
            /*1. ���ϸ� ������ ��� ����?*/
            /*2. ���� �̵���, �� ��ġ�� ��� ����? ( �Ľ� �ʿ� ) */
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " ���� �̸� ���� ����@@ ����� �� -> %wZ\n", newFileName);

        }
        // 2. ���� ���� Ȯ��
        else if (
            Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformation ||
            Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformationEx
            ) 
        {
            PFILE_DISPOSITION_INFORMATION delnameInfo = (PFILE_DISPOSITION_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
            if (delnameInfo->DeleteFile) {
                // EDR server���ѱ��
                if (Init_return__edr_data(&ProcessId, fileNameInfo, Delete, file_size, &Start_Addr, &Current_Addr) == TRUE) {
                    // ���� �� ����. ���� 'fileNameInfo'�� ǥ���ϴ� ���� ���ϸ��� ���� �� ���� 
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
        // CMD ����
        Analysis_Command cmd = File_System;
        // PID ����
        HANDLE pid = ProcessId;

        // ���� ������ ����
        PDynamicData start_node = Start_Addr;
        /*
            ���޿� ����������
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
        // ���� ����
        FltReleaseFileNameInformation(fileNameInfo);
        //return FLT_PREOP_SUCCESS_NO_CALLBACK;
        return FLT_POSTOP_FINISHED_PROCESSING;
    }
}



// PRE fix �ڵ鷯
FLT_PREOP_CALLBACK_STATUS
PRE_filter_Handler(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    // �������� Ȯ��
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
                    [BLOCK ó��]
                */

                // ���ϱ��� ���
                if (file_size == 0) {
                    ULONG32 tmp_size = 0;
                    if (!get_file_size(&fileNameInfo->Name, NULL, &tmp_size, TRUE) || tmp_size == 0) {// �ش� ������ ������ ��������
                        return FLT_PREOP_SUCCESS_NO_CALLBACK;
                    }
                    file_size = tmp_size;
                }

                // ���� �õ�
                if (is_same_check(&fileNameInfo->Name, file_size)) { // ���� ���� ���� 
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "flt ���� \n");
                    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                    Data->IoStatus.Information = 0;
                    return FLT_PREOP_COMPLETE; // Block
                }


            }
            
        }

    

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}



/*
    ���� �߻� ����: ��ȣ
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
        PASSIVE_LEVEL�� �ƴ� ��찡 �־�,  BSOD �߻����� ����, ��ȯ�۾��� �̷�������� �ȵȴ�. 
    */

    // [ EDR-Server ] File_Name ( IRQL ������ ���ڿ� ��ȯ ���� ) 
    *output_start_address_node = CreateDynamicData((PUCHAR)input_file_info->Name.Buffer, input_file_info->Name.MaximumLength);
    *output_current_address_node = *output_start_address_node;

    // ���� ����
    *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&file_size, sizeof(file_size));

    // [EDR-Server] Behavior - str ���� �ൿ
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
//�Ϲ�-�������� Check
BOOLEAN Is_File_with_Get_File_Info(
    PFLT_CALLBACK_DATA Input_Data, // �ڵ鷯���� ���� ����
    PFLT_FILE_NAME_INFORMATION* Output_fileNameInfo // ���� ��ȯ (�ƴϸ� �״�� ���� ) 
) {
    if (Output_fileNameInfo == NULL) return FALSE;

    // 1�� ���� -- ������忡 ���� ��û���� üũ
    if (Input_Data->RequestorMode != UserMode) return FALSE;

    // 2�� ���� -- �����̶�� �Ʒ��� ���� API�� ������ 
    if (FltGetFileNameInformation(Input_Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, Output_fileNameInfo) != STATUS_SUCCESS)
        return FALSE;

    // ���� �̸� ����
    if (FltParseFileNameInformation(*Output_fileNameInfo) != STATUS_SUCCESS) {

        FltReleaseFileNameInformation(*Output_fileNameInfo);
        return FALSE;
    }

    return TRUE;
}


// ���� ������ ��������
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
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Size ���� ũ�� ������ %p \n", status);
        return FALSE;
    }

    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Size ���� ũ�� ���� -> %lu \n", fileInfo.EndOfFile.QuadPart);

    *output_file_size = (ULONG32)fileInfo.EndOfFile.QuadPart;
    return TRUE;
}


// ���̳ʸ� ���� �б� ( PASSIVE_LEVEL�� �� ) 
#include "SHA256.h"
BOOLEAN Flt_Get_File_Binary(
    PFLT_CALLBACK_DATA Data,

    PUCHAR* Buffer,
    ULONG32 input_BufferSize,

    PCHAR opt_inout_SHA256
) {
    if (KeGetCurrentIrql() != PASSIVE_LEVEL || input_BufferSize > 500000000)
        return FALSE;

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Binary ȣ��� \n");
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
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Binary �ؽñ��ϱ� \n");
        GET_SHA256_HASHING(*Buffer, input_BufferSize, opt_inout_SHA256);
    }

    return TRUE;

}

VOID Flt_Get_File_Binary__Release(PUCHAR input_allocated_Buffer) {
    if(input_allocated_Buffer)
        ExFreePoolWithTag(input_allocated_Buffer, 'FLTm');
}