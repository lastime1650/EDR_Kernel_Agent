#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "File_io.h"

K_EVENT_or_MUTEX_struct file_io__mutex = { NULL, K_MUTEX, FALSE }; // �ܺ� �����忡�� ������ ��, �̸� ����ؼ� ����IO �����ؾ��Ѵ�. 

NTSTATUS Create_FILE(PHANDLE File_handle, UNICODE_STRING Create_File_Path, POBJECT_ATTRIBUTES objAttr, PIO_STATUS_BLOCK ioStatusBlock, ACCESS_MASK Desired_access, ULONG ShareOption, ULONG CreateOption) {

    NTSTATUS status = STATUS_SUCCESS;

    // 1. �Ű����� ��ȿ�� ����
    if (File_handle == NULL || Create_File_Path.Buffer == NULL || objAttr == NULL || ioStatusBlock == NULL) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE: Invalid parameter(s)\n");
        return STATUS_INVALID_PARAMETER;
    }

    // 2. �����ڵ� ���ڿ� ���� ���� (Buffer Overflow ����)
    if (Create_File_Path.Length == 0 || Create_File_Path.Length > Create_File_Path.MaximumLength) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE: Invalid Create_File_Path length\n");
        return STATUS_INVALID_PARAMETER;
    }

    // 3. OBJECT_ATTRIBUTES �ʱ�ȭ
    InitializeObjectAttributes(objAttr,
        &Create_File_Path,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL, NULL);

    // 4. ZwCreateFile ȣ�� �� ���� ó��
    __try {
        status = ZwCreateFile(File_handle,
            Desired_access,
            objAttr, ioStatusBlock, 0,
            FILE_ATTRIBUTE_NORMAL,
            ShareOption,
            CreateOption,
            FILE_SYNCHRONOUS_IO_NONALERT,
            NULL, 0);

        // 5. STATUS_INVALID_USER_BUFFER ���� ó�� (���� ����)
        //    ZwCreateFile�� ���޵Ǵ� ���۰� ����¡ ������ �޸𸮿� �ִ� ��� ProbeForRead/Write ���
        //    �� ���������� objAttr, ioStatusBlock�� Ŀ�� �޸𸮿� �ִٰ� ����
        //    Create_File_Path�� ����� ��� �ּ��� �� �����Ƿ� ProbeForRead �ʿ�
        //    ProbeForRead/Write�� ���� ���ϸ� ����ų �� �����Ƿ� �ʿ��� ��쿡�� ���
        // if (status == STATUS_INVALID_USER_BUFFER) {
        //     DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE: STATUS_INVALID_USER_BUFFER, attempting to probe memory\n");
        //     __try {
        //         ProbeForRead(Create_File_Path.Buffer, Create_File_Path.Length, sizeof(WCHAR));
        //         // objAttr, ioStatusBlock�� ����� ��� �ּҶ�� ProbeForRead/Write �ʿ�
        //         // ProbeForRead(objAttr, sizeof(OBJECT_ATTRIBUTES), sizeof(PVOID));
        //         // ProbeForWrite(ioStatusBlock, sizeof(IO_STATUS_BLOCK), sizeof(ULONG));
        //
        //         // Probe ��� �� ZwCreateFile �ٽ� ȣ��
        //         status = ZwCreateFile(File_handle,
        //             Desired_access,
        //             objAttr, ioStatusBlock, 0,
        //             FILE_ATTRIBUTE_NORMAL,
        //             ShareOption,
        //             CreateOption,
        //             FILE_SYNCHRONOUS_IO_NONALERT,
        //             NULL, 0);
        //     }
        //     __except(EXCEPTION_EXECUTE_HANDLER) {
        //         DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE: ProbeForRead/Write failed\n");
        //         status = GetExceptionCode();
        //     }
        // }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE: Exception occurred during ZwCreateFile\n");
        status = GetExceptionCode();
    }

    return status;
}

NTSTATUS Open_FILE(PHANDLE File_handle, UNICODE_STRING Create_File_Path, POBJECT_ATTRIBUTES objAttr, PIO_STATUS_BLOCK ioStatusBlock, ACCESS_MASK Desired_access, ULONG ShareOption) {


    InitializeObjectAttributes(objAttr,
        &Create_File_Path,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL, NULL);


    NTSTATUS status = ZwOpenFile(File_handle,
        Desired_access,
        objAttr, ioStatusBlock,
        ShareOption,
        FILE_SYNCHRONOUS_IO_NONALERT);


    return status;
}

/*INPUT_DATA�� �����ͷ� ���� ����*/
NTSTATUS Write_FILE(HANDLE File_handle, PVOID INPUT_DATA, ULONG INPUT_DATA_SIZE, PIO_STATUS_BLOCK ioStatusBlock) {

    NTSTATUS status = ZwWriteFile(File_handle, NULL, NULL, NULL, ioStatusBlock,
        INPUT_DATA, INPUT_DATA_SIZE, NULL, NULL);

    return status;
}
/*���� ���� ������*/
NTSTATUS Read_FILE(_In_ HANDLE File_handle, _In_ PVOID OUTPUT_DATA, _In_ ULONG inPUT_DATA_SIZE, PIO_STATUS_BLOCK ioStatusBlock) {

    NTSTATUS status = ZwReadFile(File_handle, NULL, NULL, NULL, ioStatusBlock,
        OUTPUT_DATA, inPUT_DATA_SIZE, NULL, NULL);

    return status;
}



NTSTATUS ALL_in_ONE_FILE_IO(PVOID* RAW_DATA, ULONG* RAW_DATA_SIZE, UNICODE_STRING File_path, ALL_in_ONE_FILE_IO_info custom_MODE) {

    NTSTATUS status = STATUS_SUCCESS;

    HANDLE filehandle = 0;


    OBJECT_ATTRIBUTES objAttr = { 0, };
    IO_STATUS_BLOCK ioStatusBlock = { 0, };




    switch (custom_MODE) {
    case    WRITE_MODE: //���� �� ��
        status = Create_FILE(&filehandle, File_path, &objAttr, &ioStatusBlock, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OVERWRITE_IF);
        if (status != STATUS_SUCCESS) {
            // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE - WRITE -> Create_FILE ��� %p ", status);
            return status;
        }

        status = Write_FILE(filehandle, *RAW_DATA, *RAW_DATA_SIZE, &ioStatusBlock);
        if (status != STATUS_SUCCESS) {
            ZwClose(filehandle);
            // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Write_FILE ->  ��� %p ", status);
            return status;
        }
        break;



    case    READ_MODE: //�б� �� ��
        status = Create_FILE(&filehandle, File_path, &objAttr, &ioStatusBlock, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN);
        if (status != STATUS_SUCCESS) {
            //  DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE - READ -> Create_FILE ��� %p ", status);
            return status;
        }

        status = Read_FILE(filehandle, *RAW_DATA, *RAW_DATA_SIZE, &ioStatusBlock);
        if (status != STATUS_SUCCESS) {
            ZwClose(filehandle);
            // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Read_FILE ->  ��� %p\n ", status);
            return status;
        }
        break;


        // �Ʒ� ����� �� ENUM ��,,, FULL-PATH�� ������ ���� EXE ���̳ʸ��� ���������� �Ѵ�. 
    case    READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH: // ���� ���� ���� ��
        //status = Create_FILE(&filehandle, File_path, &objAttr, &ioStatusBlock, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE , FILE_OPEN);
        //status = Open_FILE(&filehandle, File_path, &objAttr, &ioStatusBlock, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
        //if (status != STATUS_SUCCESS) {
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE - READ -> Create_FILE ��� %p ", status);
        //    return status;
       // }
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���� ���̳ʸ� ��û [���]:%wZ \n", File_path );
        status = Create_FILE(&filehandle, File_path, &objAttr, &ioStatusBlock, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN);
        if (status != STATUS_SUCCESS) {
            //  DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE - READ -> Create_FILE ��� %p ", status);
            return status;
        }
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "1 ���� ���̳ʸ� ��û Create_FILE ���� \n");

        /*���� STANDARD ���*/
        FILE_STANDARD_INFORMATION fileStandardInfo; // ���� ��ü ��û ����
        status = ZwQueryInformationFile(filehandle, &ioStatusBlock, &fileStandardInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
        if (status != STATUS_SUCCESS) {
            ZwClose(filehandle);
           // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE - READ -> ZwQueryInformationFile ��� %p ", status);
            return STATUS_UNSUCCESSFUL;
        }
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "2 ���� ���̳ʸ� ��û ZwQueryInformationFile ���� \n");
        /*���� ���� ����*/
        *RAW_DATA_SIZE = (ULONG)fileStandardInfo.EndOfFile.QuadPart;

        /* ���� ���̿� ���� �����Ҵ� <- �̰����� EXE ���α׷��� ���*/
        *RAW_DATA = ExAllocatePoolWithTag(PagedPool, *RAW_DATA_SIZE, 'FILE'); // ���� �Ҵ� �õ��� �ϹǷ�, NonPaged�� �ϸ� ��罺ũ�� -> �޸� �̽�
        if (*RAW_DATA == NULL) {
            ZwClose(filehandle);
            return STATUS_UNSUCCESSFUL;
        }
        memset(*RAW_DATA, 0, *RAW_DATA_SIZE); // 0���� set


        status = Read_FILE(filehandle, *RAW_DATA, *RAW_DATA_SIZE, &ioStatusBlock);
        if (status != STATUS_SUCCESS) {
            ZwClose(filehandle);
           // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Read_FILE ->  ��� %p ", status);
            return status;
        }

        break;  

    default:
        return STATUS_UNSUCCESSFUL;
    }
    ZwClose(filehandle);
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���� ��û ���� ���� \n");
    return status;
}




/*
    ��ƿ
*/
#include "SHA256.h"
#include "DynamicData_Linked_list.h"
#include "PE_Manager.h"
#include "query_process_information.h"
BOOLEAN FILE_to_INFO(
    UNICODE_STRING* file_path,
    HANDLE* opt_pid,
    ULONG32* output_file_size,
    PE_info* output_pe_info,
    CHAR* output_sha256
) {
    BOOLEAN return_bool = TRUE;
    K_object_init_check_also_lock_ifyouwant(&file_io__mutex, TRUE);

    // 0 PID�� ���� ���, ������ ��θ� �����Ѵ�.
    if (opt_pid != NULL) {
        HANDLE PID = *opt_pid;
        // PID ���� EXE ������ ���
        if (Query_Process_info(PID, ProcessImageFileName, &file_path) != STATUS_SUCCESS) {
            K_object_lock_Release(&file_io__mutex);
            return FALSE;
        }
        // ����
    }
    

    // 1. ���� ���� + ������ ����
    PUCHAR File_Buffer = NULL;
    ULONG File_Size = 0;
    if (ALL_in_ONE_FILE_IO(&File_Buffer, &File_Size, *file_path, READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH) != STATUS_SUCCESS) {
        return_bool = FALSE;
        if (opt_pid != NULL) {
            goto EXIT2;
        }
        else {
            goto EXIT;
        }
        
    }

    if (output_file_size) {
        *output_file_size = (ULONG32)File_Size;
    }


    if (output_pe_info) {
        
        PIMAGE_DOS_HEADER_c dosHeader = NULL;
        PIMAGE_NT_HEADERS_c ntHeaders = NULL;
        PIMAGE_FILE_HEADER_c fileHeader = NULL;
        PIMAGE_OPTIONAL_HEADER32_c optionalHeader32 = NULL;
        PIMAGE_OPTIONAL_HEADER64_c optionalHeader64 = NULL;
        ULONG32 PE_Architecture = 0;
        PDynamicData Output_Optional_Directories = NULL;
        ExtractPEHeaders(File_Buffer, &dosHeader, &ntHeaders, &fileHeader, &optionalHeader32, &optionalHeader64, &PE_Architecture, &Output_Optional_Directories);

        ULONG32 Output_CertificateType = 0;
        Check_Digital_Certificate_info(File_Buffer, Output_Optional_Directories, &Output_CertificateType);
        Release_IMAGE_DATA_DIRECTORY_c(Output_Optional_Directories);

        output_pe_info->dosHeader = dosHeader;
        output_pe_info->ntHeaders= ntHeaders;
        output_pe_info->fileHeader= fileHeader;
        output_pe_info->optionalHeader32= optionalHeader32;
        output_pe_info->optionalHeader64= optionalHeader64;
        output_pe_info->PE_Architecture= PE_Architecture;
        output_pe_info->Output_CertificateType = Output_CertificateType;

    }

    if (output_sha256) {
        // 2. SHA256
        if (GET_SHA256_HASHING(File_Buffer, File_Size, output_sha256) != STATUS_SUCCESS) {
            return_bool = FALSE;
        }
        else {
            return_bool = TRUE;
        }
    }

    if (opt_pid != NULL) {
        goto EXIT2;
    }
    else {
        goto EXIT;
    }


EXIT2:
    {
        Query_Process_Image_Name___Release_Free_POOL(file_path); // PID -> filepath ���Ѱ�쿡�� ȣ���ؾ��Ѵ�!!
        goto EXIT;
    }

EXIT:
    {
        if (File_Buffer)
            ExFreePoolWithTag(File_Buffer, 'FILE');

        K_object_lock_Release(&file_io__mutex);
        return return_bool;
    }
}


// ���� ���� üũ
BOOLEAN get_file_size(
    UNICODE_STRING* opt_file_path,
    HANDLE* opt_PID,

    ULONG32* OUTPUT_knwon_file_size
) {
    if (KeGetCurrentIrql() != PASSIVE_LEVEL && KeGetCurrentIrql() != APC_LEVEL)
        return FALSE;

    BOOLEAN output = TRUE;

    // 0 PID�� ���� ���, ������ ��θ� �����Ѵ�.
    if (opt_PID != NULL) {
        HANDLE PID = *opt_PID;
        // PID ���� EXE ������ ���
        if (Query_Process_info(PID, ProcessImageFileName, &opt_file_path) != STATUS_SUCCESS) {
            return FALSE;
        }
        // ����
       // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [TEST]-get_file_size-Query_Process_info-���� \n");
    }

    // 1. ���� ���� ����
    HANDLE filehandle = 0;
    OBJECT_ATTRIBUTES objAttr = { 0, };
    IO_STATUS_BLOCK ioStatusBlock = { 0, };
    if (Create_FILE(&filehandle, *opt_file_path, &objAttr, &ioStatusBlock, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN) != STATUS_SUCCESS) {
        output = FALSE;
        if (opt_PID != NULL) {
           // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [TEST]-get_file_size-Create_FILE-���� \n");
            goto EXIT2;
        }
        else {
            goto EXIT;
        }
    }

    /*���� STANDARD ���*/
    FILE_STANDARD_INFORMATION fileStandardInfo; // ���� ��ü ��û ����
    if (ZwQueryInformationFile(filehandle, &ioStatusBlock, &fileStandardInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation) != STATUS_SUCCESS) {
        output = FALSE;
        ZwClose(filehandle);
        if (opt_PID != NULL) {
            goto EXIT2;
        }
        else {
            goto EXIT;
        }
    }
   // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [TEST]-get_file_size-ZwQueryInformationFile-%wZ ������:%lu\n", opt_file_path,(ULONG32)fileStandardInfo.EndOfFile.QuadPart);
    /*���� ���� ����*/
    *OUTPUT_knwon_file_size = (ULONG32)fileStandardInfo.EndOfFile.QuadPart;
    output = TRUE;

    ZwClose(filehandle);
    if (opt_PID != NULL) {
        goto EXIT2;
    }
    else {
        goto EXIT;
    }

EXIT2:
    {
        Query_Process_Image_Name___Release_Free_POOL(opt_file_path); // PID -> filepath ���Ѱ�쿡�� ȣ���ؾ��Ѵ�!!
        goto EXIT;
    }

EXIT:
    {
        return output;
    }
}