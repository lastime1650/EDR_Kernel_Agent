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

    BOOLEAN is_DENIED,

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


/*
=========
*/

#include "WorkItem_job.h"
#include "Response_File.h"
#include "File_io.h"

// Post-Operation�� ������ ���ؽ�Ʈ ����ü ���� (����)
typedef struct _PRE_2_POST_CONTEXT {
    BOOLEAN IsDeleteOperation;
    // �ʿ��ϴٸ� ���� �̸� ���� �߰� ���� ����
    // PUNICODE_STRING FileName;
} PRE_2_POST_CONTEXT, * PPRE_2_POST_CONTEXT;

// PRE fix �ڵ鷯
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

    // �ý��۰��� ���μ������� Ȯ��
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

    // �������� Ȯ��
    PFLT_FILE_NAME_INFORMATION fileNameInfo = NULL;
    if (Is_File_with_Get_File_Info(Data, &fileNameInfo) == FALSE) {
        Release_Got_Time(Timestamp);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    BOOLEAN is_Denied_RETURN = FALSE; // ���� ���� ����

    ULONG32 file_size = 0;

    // 3. ZwCreateFile�� ����Ͽ� ���� ����
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
        FILE_GENERIC_READ | SYNCHRONIZE, // �ּ����� ���Ѹ� ��û
        &objAttr,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ, // ���� ���
        FILE_OPEN, // �̹� �����ϴ� ���ϸ� ����
        FILE_SYNCHRONOUS_IO_NONALERT, // ���� I/O
        NULL,
        0
    );
    if (status != STATUS_SUCCESS) {
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[PRE-ZwCreateFile] ����\n");
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
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���� ���� ���ߴµ�? ���: %wZ // ����: %lu \n", fileNameInfo->Name, file_size);
    }

    ZwClose(fileHandle);
    
    // [ TEST ] ���̰� 0 �� ���� ��޾���
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
        ������ �����ϱ� ���� ��� �غ�
    */
    PDynamicData Start_Addr = NULL;
    PDynamicData Current_Addr = NULL;


    // IRP �� ó��
    switch (Data->Iopb->MajorFunction) {
    case IRP_MJ_CREATE:
    {

        // ���� �õ�
        if (is_same_check(&fileNameInfo->Name, file_size)) { // ���� ���� ���� 
            /*
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "flt ���� \n");
            Data->IoStatus.Status = STATUS_ACCESS_DENIED;
            Data->IoStatus.Information = 0;
            FltReleaseFileNameInformation(fileNameInfo);
            return FLT_PREOP_COMPLETE; // Block
            */
            goto DENIED_return;
        }




        //���������� �ʱ�ȭ
        memset(share_filter_object, 0, sizeof(Share_filter_Obj) - sizeof(KSPIN_LOCK));


        

        // Create ( ���� ���� Ȯ�� )
        if (Data->Iopb->Parameters.Create.Options & FILE_CREATE) {
            // ���� ����

            if (Init_return__edr_data(&ProcessId, fileNameInfo, Create, file_size, is_Denied_RETURN , &Start_Addr, &Current_Addr) == TRUE) { // �̰��� �ѹ��� �׽�Ʈ�ؾ��� 
               // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Pre-CREATE]�̴����� ���ϰ��: %wZ / SecContext: %d / Options: %d \n", fileNameInfo->Name, Data->Iopb->Parameters.Create.SecurityContext, Data->Iopb->Parameters.Create.Options);
                goto To_Server;

            }
        }
        else {
            // ���� ����

            // ���� ���� Ȯ��
            if (Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & (FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES | GENERIC_WRITE)) {
                // ���� ���� ���� ����
               // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Pre-WRITE]�̴����� ���ϰ��: %wZ / SecContext: %d / Options: %d \n", fileNameInfo->Name, Data->Iopb->Parameters.Create.SecurityContext, Data->Iopb->Parameters.Create.Options);
                KIRQL oldIrql;
                KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
                share_filter_object->IsWriteMode = TRUE;
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�̴����� [PID]: %llu ���� ���� ���� ���� \n", ProcessId);
            }

            // �б� ���� Ȯ�� (���� ���Ѱ� ������)
            if (Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & (FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA | GENERIC_READ)) {
               // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Pre-READ]�̴����� ���ϰ��: %wZ / SecContext: %d / Options: %d \n", fileNameInfo->Name, Data->Iopb->Parameters.Create.SecurityContext, Data->Iopb->Parameters.Create.Options);
                // ���� �б� ���� ����
                KIRQL oldIrql;
                KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
                share_filter_object->IsReadMode = TRUE;
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�̴����� [PID]: %llu ���� �б� ���� ���� \n", ProcessId);
            }
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�̴����� [PID]: %llu ����:%d / �б�:%d \n", ProcessId, share_filter_object->IsWriteMode, share_filter_object->IsReadMode);

            // ���� ���� Ȯ��
            if (Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & DELETE) {
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Pre-DELETE]�̴����� ���ϰ��: %wZ / SecContext: %d / Options: %d \n", fileNameInfo->Name, Data->Iopb->Parameters.Create.SecurityContext, Data->Iopb->Parameters.Create.Options);
            }
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

            if (Init_return__edr_data(&ProcessId, fileNameInfo, Write, file_size, is_Denied_RETURN, &Start_Addr, &Current_Addr) == TRUE) {
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[WRITE]�̴����� ���ϰ��: %wZ  \n", fileNameInfo->Name);
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                goto To_Server;

            }
        }
        KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);

        break;
    }
    case IRP_MJ_READ:
    {

        // ���� �б�
        KIRQL oldIrql;
        KeAcquireSpinLock(&share_filter_object->spinlock, &oldIrql);
        if (share_filter_object->IsReadMode) {
            

            if (Init_return__edr_data(&ProcessId, fileNameInfo, Read, file_size, is_Denied_RETURN, &Start_Addr, &Current_Addr) == TRUE) {
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[READ]�̴����� ���ϰ��: %wZ  \n", fileNameInfo->Name);
                KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
                goto To_Server;

            }

            
        }
        KeReleaseSpinLock(&share_filter_object->spinlock, oldIrql);
        break;

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
                if (Init_return__edr_data(&ProcessId, fileNameInfo, Rename, file_size, is_Denied_RETURN, &Start_Addr, &Current_Addr) == TRUE) {
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Rename]�̴����� ���ϰ��: %wZ  \n", fileNameInfo->Name);
                    // ����Ǵ� �̸� �߰� ( PWCH )
                    Current_Addr = AppendDynamicData(Current_Addr, (PUCHAR)renameInfo->FileName, renameInfo->FileNameLength);

                    // 

                    goto To_Server;
                }



            }
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
                if (Init_return__edr_data(&ProcessId, fileNameInfo, Delete, file_size, is_Denied_RETURN, &Start_Addr, &Current_Addr) == TRUE) {
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[DELETE]�̴����� ���ϰ��: %wZ  \n", fileNameInfo->Name);
                    // ���� �� ����. ���� 'fileNameInfo'�� ǥ���ϴ� ���� ���ϸ��� ���� �� ���� 
                    //is_Denied_RETURN = TRUE; // TRUE�� ����
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
        // ���� ����
        FltReleaseFileNameInformation(fileNameInfo);
        //return FLT_PREOP_SUCCESS_NO_CALLBACK;
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

DENIED_return:
    {
        // ���� ���� ����
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


    // [ EDR-Server ] File_Name ( IRQL ������ ���ڿ� ��ȯ ���� ) 
    *output_start_address_node = CreateDynamicData((PUCHAR)input_file_info->Name.Buffer, input_file_info->Name.MaximumLength);
    *output_current_address_node = *output_start_address_node;

    // ���� ����
    *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&file_size, sizeof(file_size));

    // + ���� �̷µ� Ȯ���Ѵ�
    if (is_DENIED) {
        UCHAR DENIED[] = "DENIED";
        *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&DENIED, sizeof(DENIED) - 1);
    }
    else {
        UCHAR GRANTED[] = "GRANTED";
        *output_current_address_node = AppendDynamicData(*output_current_address_node, (PUCHAR)&GRANTED, sizeof(GRANTED) - 1);
    }



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
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Size ���� ũ�� ������ %p \n", status);
        return FALSE;
    }

    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Size ���� ũ�� ���� -> %lu \n", fileInfo.EndOfFile.QuadPart);

    *output_file_size = (ULONG32)fileInfo.EndOfFile.QuadPart;
    return TRUE;
}

/*
// ���̳ʸ� ���� �б� ( PASSIVE_LEVEL�� �� ) 
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
        NTSTATUS status = GET_SHA256_HASHING(*Buffer, input_BufferSize, opt_inout_SHA256);
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Flt_Get_File_Binary �ؽñ��ϱ� -> %p \n", status);
    }

    return TRUE;

}

VOID Flt_Get_File_Binary__Release(PUCHAR input_allocated_Buffer) {
    if(input_allocated_Buffer)
        ExFreePoolWithTag(input_allocated_Buffer, 'FLTm');
}*/

/*
#define CHUNK_READ_SIZE 4096 // �� ���� ���� ûũ ũ�� (��: 4KB)
#define CHUNK_POOL_TAG 'RtsP' // �޸� �Ҵ� �±� (��: 'PostR'ead)



FLT_POSTOP_CALLBACK_STATUS
PostSetInfoCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext, // Pre-Op���� ���޵� ���ؽ�Ʈ
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{

    // �ý��۰��� ���μ������� Ȯ��
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

    // 1. Draining ���� Ȯ��: ���� ��ε� �߿��� ���� I/O ����
    if (FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING)) {
        // ��ε� ���̹Ƿ� �߰� �۾� ���� ����
        // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "PostReadCallback: Filter is draining, skipping chunk read.\n"); // ���� ������ ���� ����
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    // 2. ���� Read �۾� ���� ���� Ȯ�� (���� ���������� ����)
    //    - ������ Read �Ŀ� ���⼭ �ٽ� �д� ���� �ǹ̰� ���ų� ������ �� ����
    //    - NT_SUCCESS�� STATUS_END_OF_FILE�� ������ �� ������ ����
    if (!NT_SUCCESS(Data->IoStatus.Status) && Data->IoStatus.Status != STATUS_END_OF_FILE) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: Original read failed (0x%X), skipping chunk read.\n", Data->IoStatus.Status);
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    // 3. ��ȿ�� ��ü Ȯ��
    if (FltObjects == NULL || FltObjects->FileObject == NULL || FltObjects->Instance == NULL) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: Invalid FltObjects.\n");
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: Starting chunk read for FileObject %p\n", FltObjects->FileObject); // ���� ������ ���� ����
    
    // 4. ûũ �б⸦ ���� ���� �Ҵ�
    chunkBuffer = ExAllocatePoolZero(POOL_FLAG_NON_PAGED, CHUNK_READ_SIZE, CHUNK_POOL_TAG);
    if (chunkBuffer == NULL) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: Failed to allocate chunk buffer.\n");
        // ���� �Ҵ� ���� �ÿ��� ���� �۾� ����� �������� ����
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    // 5. ���� ���ۺ��� ûũ ������ �б� ����
    readOffset.QuadPart = 0; // ���� ���� ������
    do {
        // FltReadFile ȣ���Ͽ� ûũ �б�
        status = FltReadFile(
            FltObjects->Instance,       // ���� �ν��Ͻ�
            FltObjects->FileObject,     // ��� ���� ��ü
            &readOffset,                // �б� ���� ��ġ (���� ���ۺ��� ����)
            CHUNK_READ_SIZE,            // ���� ����Ʈ �� (ûũ ũ��)
            chunkBuffer,                // �����͸� ������ ����
            FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET, // FileObject�� ���� ��ġ�� �������� ����
            &bytesActuallyRead,         // ������ ���� ����Ʈ ��
            NULL,                       // ���� ȣ���̹Ƿ� �ݹ� ����
            NULL                        // �ݹ� ���ؽ�Ʈ ����
        );

        if (NT_SUCCESS(status)) {
            if (bytesActuallyRead > 0) {
                // ���������� ûũ�� ����
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: Read %u bytes at offset %lld\n", bytesActuallyRead, readOffset.QuadPart); // ���� ������ ���� ����

                // ==================================================
                // ���⿡ ���� ûũ(chunkBuffer)�� ���� ó�� ���� �߰�
                // ��: ���� �˻�, �ؽ� ���, �α� ��� ��
                // ProcessChunkData(chunkBuffer, bytesActuallyRead);
                // ==================================================

                // ���� �б� ��ġ�� ������ �̵�
                readOffset.QuadPart += bytesActuallyRead;
            }
            else {
                // ���������� ���� ����Ʈ�� 0 -> ���� ��(EOF) ����
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: EOF reached during chunk read.\n"); // ���� ������ ���� ����
                break; // ���� ����
            }
        }
        else if (status == STATUS_END_OF_FILE) {
            // ��������� EOF ���� ��ȯ��
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: FltReadFile returned STATUS_END_OF_FILE.\n"); // ���� ������ ���� ����
            bytesActuallyRead = 0; // ���� ���� ���� ������Ű�� ����
            break; // ���� ����
        }
        else {
            // �б� �� ���� �߻�
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: FltReadFile failed with status 0x%X at offset %lld\n", status, readOffset.QuadPart);
            // ���⼭�� ������ ��ϸ� �ϰ� ���� ����. ���� I/O ���´� �������� ����.
            break; // ���� ����
        }

        // ���� ����Ʈ�� �ְ�, ûũ ũ�⸸ŭ �о��� �� �����Ƿ� ��� �õ�
    } while (bytesActuallyRead > 0); // ���� ���� ����Ʈ�� ���� ������ �ݺ�

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PostReadCallback: Finished chunk read.\n"); // ���� ������ ���� ����

    // 6. �Ҵ��ߴ� ���� ���� (���� ���� �� �ݵ�� ����)
    if (chunkBuffer != NULL) {
        ExFreePoolWithTag(chunkBuffer, CHUNK_POOL_TAG);
    }

    // 7. Post-Operation ó�� �Ϸ� ��ȯ
    //    - ���⼭ �߻��� ������ ���� ���� IRP�� ���¸� �������� ����
    return FLT_POSTOP_FINISHED_PROCESSING;
}*/