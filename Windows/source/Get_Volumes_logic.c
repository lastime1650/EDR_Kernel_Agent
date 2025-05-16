#include "Get_Volumes.h"

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, * POBJECT_DIRECTORY_INFORMATION;
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryDirectoryObject(
    _In_ HANDLE DirectoryHandle,
    _Out_opt_ PVOID Buffer,
    _In_ ULONG Length,
    _In_ BOOLEAN ReturnSingleEntry,
    _In_ BOOLEAN RestartScan,
    _Inout_ PULONG Context,
    _Out_opt_ PULONG ReturnLength
);


POBJECT_TYPE* IoDriverObjectType;
K_EVENT_or_MUTEX_struct Get_Volume_KMUTEX = {NULL, K_MUTEX, FALSE};

VOID ListMountedDrives(PUNICODE_STRING PNP_Device_Name_from_PNP_UNICODE_STRING, BOOLEAN is_remove_node)
{
    K_object_init_check_also_lock_ifyouwant(&Get_Volume_KMUTEX, TRUE);

    is_remove_node;

    UNICODE_STRING uniString;
    OBJECT_ATTRIBUTES objAttr;
    HANDLE dirHandle;
    NTSTATUS status;
    PVOID buffer;
    ULONG context = 0;
    ULONG returnedLength;
    POBJECT_DIRECTORY_INFORMATION dirInfo;
    UNICODE_STRING ntPathPrefix = RTL_CONSTANT_STRING(L"\\Device\\");

    RtlInitUnicodeString(&uniString, L"\\??");
    InitializeObjectAttributes(&objAttr, &uniString, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    status = ZwOpenDirectoryObject(&dirHandle, DIRECTORY_QUERY, &objAttr);
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to open directory object: %x\n", status);
        K_object_lock_Release(&Get_Volume_KMUTEX);
        return;
    }

    buffer = ExAllocatePoolWithTag(PagedPool, 1024 * 1024, 'dirb');
    if (!buffer) {
        ZwClose(dirHandle);
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to allocate buffer\n");
        K_object_lock_Release(&Get_Volume_KMUTEX);
        return;
    }

    while (TRUE) {
        status = ZwQueryDirectoryObject(dirHandle, buffer, 1024 * 1024, TRUE, FALSE, &context, &returnedLength);
        if (status == STATUS_NO_MORE_ENTRIES) {
            break;
        }

        if (!NT_SUCCESS(status)) {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to query directory object: %x\n", status);
            break;
        }

        dirInfo = (POBJECT_DIRECTORY_INFORMATION)buffer;

        for (; ; dirInfo++) {
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "dirInfo -> %d\n", dirInfo);
            if (dirInfo->Name.Length == 0) {
                break;
            }

            HANDLE linkHandle;
            UNICODE_STRING linkTarget;
            WCHAR targetBuffer[MAXIMUM_FILENAME_LENGTH];
            linkTarget.Buffer = targetBuffer;
            linkTarget.Length = 0;
            linkTarget.MaximumLength = sizeof(targetBuffer);

            InitializeObjectAttributes(&objAttr, &dirInfo->Name, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, dirHandle, NULL);
            status = ZwOpenSymbolicLinkObject(&linkHandle, SYMBOLIC_LINK_QUERY, &objAttr);
            if (NT_SUCCESS(status)) {
                status = ZwQuerySymbolicLinkObject(linkHandle, &linkTarget, NULL);
                if (NT_SUCCESS(status)) {
                    if (RtlPrefixUnicodeString(&ntPathPrefix, &linkTarget, TRUE)) {
                        //

                        DEVICE_DECTECT_ENUM result = is_external_Device(&dirInfo->Name, &linkTarget, PNP_Device_Name_from_PNP_UNICODE_STRING);

                        switch (result) {
                        case JUST_VOLUME_DISK:
                        {
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Mounted JUST_VOLUME_DISK Drive: %wZ -> %wZ\n", &dirInfo->Name, &linkTarget);
                            break;
                        }
                        case Internal_DISK:
                        {
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Mounted Internal_DISK Drive: %wZ -> %wZ\n", &dirInfo->Name, &linkTarget);
                            break;
                        }
                        case External_DISK_USB:
                        {
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Mounted External_DISK_USB Drive: %wZ -> %wZ\n", &dirInfo->Name, &linkTarget);
                            break;
                        }
                        case USB_DEVICE_from_PNP:
                        {
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Mounted USB Drive: %wZ -> %wZ\n", &dirInfo->Name, &linkTarget);
                            break;
                        }
                            
                        default:
                            NULL;
                            break;
                        }
                        if (result != DEVICE_None && result != USB_DEVICE_from_PNP && result != External_DISK_USB) {
                            // �������� ���� �� ���� UPDATE


                            PALL_DEVICE_DRIVES Node_from_Finder = NULL;

                            //if (is_PNP_call == FALSE) {
                            Node_from_Finder = Finder_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node, &linkTarget, NULL, NULL);
                            // }
                             //else {
                                 //Node_from_Finder = Finder_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node, &linkTarget, &result);
                             //}




                            if (Node_from_Finder == NULL) {
                                // ���� ��忡 ���� �� ���� ��� ����
                                if (ALL_DEVICE_DRIVES_Start_Node == NULL) {

                                    ALL_DEVICE_DRIVES_Start_Node = Create_ALL_DEVICE_DRIVES_Node(NULL, &dirInfo->Name, &linkTarget, NULL, &result);

                                    ALL_DEVICE_DRIVES_Current_Node = ALL_DEVICE_DRIVES_Start_Node;
                                }
                                else {
                                    ALL_DEVICE_DRIVES_Current_Node = Append_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Current_Node, &dirInfo->Name, &linkTarget, NULL, &result);
                                }


                            }
                            else {
                                // �̹����� ��, UPDATE��
                                Update_ALL_DEVICE_DRIVES_Node(Node_from_Finder, &dirInfo->Name, &result);
                            }


                            //Print_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node);


                        }
                        Complete_ALL_DEVICE_DRIVED_Node(&ALL_DEVICE_DRIVES_Start_Node, &ALL_DEVICE_DRIVES_Current_Node);

                    }

                    /*
                        USB���� Ȯ��.
                    */



                }
                ZwClose(linkHandle);
            }
        }
    }

    K_object_lock_Release(&Get_Volume_KMUTEX);
    ExFreePoolWithTag(buffer, 'dirb');
    ZwClose(dirHandle);

    Print_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node);
}


// USB���� �����ϵ����� ... ENUM���� ��ȯ
DEVICE_DECTECT_ENUM is_external_Device(PUNICODE_STRING Obj_Dir_NAME, PUNICODE_STRING NT_NAME, PUNICODE_STRING PNP_Device_Name_from_PNP_UNICODE_STRING) {
    PNP_Device_Name_from_PNP_UNICODE_STRING;
    // ���͸� �����ڵ��
    UNICODE_STRING filter_string[] = {
        {0},
        {0}
    };
    RtlInitUnicodeString(&filter_string[0], L"STORAGE#Volume#{"); // �ϵ��ũ
    RtlInitUnicodeString(&filter_string[1], L"STORAGE#Volume#_??_USBSTOR#"); // USB ����

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " �ϱ��� %wZ  / NT: %wZ \n", Obj_Dir_NAME, NT_NAME);
    // ����Ʈ�� ��ġ���� ( ���ĺ�Volume�� ���� �͵��̶�� ��� �ش�ȴ�. ) 
    if (Obj_Dir_NAME->Length == 4 && Obj_Dir_NAME->Buffer[1] == L':' &&
        ((Obj_Dir_NAME->Buffer[0] >= L'A' && Obj_Dir_NAME->Buffer[0] <= L'Z') ||
            (Obj_Dir_NAME->Buffer[0] >= L'a' && Obj_Dir_NAME->Buffer[0] <= L'z'))) {

        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Mounted_MSSS_DOSSS Drive: %wZ -> %wZ\n", *Obj_Dir_NAME, *NT_PATH);

        /*
            ���ĺ�������, ���� ������ �ʿ��� �κ��� ����. 
        */
        
        ULONG32 device_after_index = 8; // L"\Device\{����!}" �ε��� {����!} <- �� �ǹ�

        // ISO���� ����
        if (
            NT_NAME->Length >= (device_after_index + 1 + 4) &&
            (wcsncmp(&NT_NAME->Buffer[device_after_index], L"CdRom",  (sizeof(L"CdRom")/sizeof(WCHAR))-1 )==0)
            )
            return External_DISK_ISO;
        

        return JUST_VOLUME_DISK;
    }

    // �����ϵ�����
    if (memcmp(Obj_Dir_NAME->Buffer, filter_string[0].Buffer, sizeof(L"STORAGE#Volume#{") - sizeof(WCHAR)) == 0) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Internal_DISK �Դϴ�.\n");
        return Internal_DISK;
    }


    // USB �����ϵ� ���� Ȯ�� ( �̶��� ���� ���� "��" �� )
    PWCH filter = L"USB#";
    PWCH filter2 = L"\\??\\USB#"; // PNP���� ������ �ɺ��� ��ũ

    // USB#�� �����ϴ��� Ȯ��
    if (
        (memcmp(Obj_Dir_NAME->Buffer, filter, sizeof(L"USB#") - sizeof(WCHAR)) == 0) ||
        (memcmp(Obj_Dir_NAME->Buffer, filter2, sizeof(L"\\??\\USB#") - sizeof(WCHAR)) == 0)

        ) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PNP���κ����� USB�̸�, ������ ��� �� �Դϴ�.\n");
        /*
            �� ������  ����Ʈ�� ��ġ�� ������ ��� ���̹Ƿ�

            1. �ø��� ����
            2. ��忡 �ø��� ������� Ž��
            3. ��� ������ �ø��� ���� ��� ����

             ����� ��κ� PNP���� ���Ǵ� ����.
            4. �ʿ�� ������Ʈ
        */

        UNICODE_STRING get_SERIAL_NUM = { 0, }; // USB ����̽��κ��� �ø����� ������ ������ 
        DEVICE_DECTECT_ENUM result = USB_DEVICE_from_PNP;
        if (PNP_Device_Name_from_PNP_UNICODE_STRING != NULL) {
            /*
                ���⼭ ���� PNP �ݹ��Լ��� �񵿱� �����忡�� ȣ���� ���

            */
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [�ø���] USB_DEVICE_from_PNP memcmp( %wZ vs %wZ )\n", PNP_Device_Name_from_PNP_UNICODE_STRING, NT_NAME);
            if (memcmp(PNP_Device_Name_from_PNP_UNICODE_STRING->Buffer, NT_NAME->Buffer, PNP_Device_Name_from_PNP_UNICODE_STRING->Length) == 0) {
                if (Get_USB_Serial(Obj_Dir_NAME, &get_SERIAL_NUM, 2, FALSE)) {// �ø��� ���� �� �ø���ȿ� "&" �� ����, �ʿ���� 
                    /*
                       USB �ø��� ���������� ���� ��
                   */
                    PALL_DEVICE_DRIVES Node_from_Finder = NULL;
                    Node_from_Finder = Finder_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node, NULL, &get_SERIAL_NUM, NULL);
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [�ø���] Finder_ALL_DEVICE_DRIVES_Node -> %p\n", Node_from_Finder);
                    if (Node_from_Finder == NULL) {
                        // ���� ��忡 ���� �� ���� ��� ����
                        if (ALL_DEVICE_DRIVES_Start_Node == NULL) {

                            ALL_DEVICE_DRIVES_Start_Node = Create_ALL_DEVICE_DRIVES_Node(NULL, Obj_Dir_NAME, NT_NAME, &get_SERIAL_NUM, &result);

                            ALL_DEVICE_DRIVES_Current_Node = ALL_DEVICE_DRIVES_Start_Node;
                        }
                        else {
                            ALL_DEVICE_DRIVES_Current_Node = Append_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Current_Node, Obj_Dir_NAME, NT_NAME, &get_SERIAL_NUM, &result);
                        }


                    }
                    else {
                        // �̹����� ��, UPDATE��
                        //Update_ALL_DEVICE_DRIVES_Node(Node_from_Finder, &dirInfo->Name, &result);
                    }
                    //////////////Print_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node);

                }
            }






        }
        return USB_DEVICE_from_PNP;
    }



    // USB �����ϵ� ���� Ȯ�� ( �̶��� ���� ���� "��" �� )
    if (memcmp(Obj_Dir_NAME->Buffer, filter_string[1].Buffer, sizeof(L"STORAGE#Volume#_??_USBSTOR#") - sizeof(WCHAR)) == 0) {
        /*
            �� ������ �̹� ����Ʈ�� ��ġ�� ������ ���� ���̱� ������

            1. �ø��� ����
            2. ��忡 �ø��� ������� Ž��
            3. ��� ������ �ø��� ���� ��� ����
            4. ��� ������ ������Ʈ

        */
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "External_DISK_USB �Դϴ�. %wZ -> %wZ\n", *Obj_Dir_NAME, *NT_NAME);
        /*
            �ø��� ���� �� �̹� �ø����� ��忡 �ִ� ��� UPDATE / ������ ����
        */
        UNICODE_STRING get_SERIAL_NUM;
        DEVICE_DECTECT_ENUM result = External_DISK_USB;
        if (Get_USB_Serial(Obj_Dir_NAME, &get_SERIAL_NUM, 4, TRUE)) {// �ø��� ���� �� �ø���ȿ� "&" �� �߰��� �ִٸ�, ���͸���
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " ���� USB: %wZ \n", get_SERIAL_NUM);



            PALL_DEVICE_DRIVES Node_from_Finder = NULL;
            Node_from_Finder = Finder_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node, NULL, &get_SERIAL_NUM, NULL);
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [�ø���] Finder_ALL_DEVICE_DRIVES_Node ///  External_DISK_USB -> %p\n", Node_from_Finder);
            if (Node_from_Finder == NULL) {

                // NT ��η� ��Ž��
                Node_from_Finder = Finder_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node, NT_NAME, NULL, NULL);
                if (Node_from_Finder == NULL) {
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [�ø���] �ι����� Ʋ��\n");
                    // ���� ��忡 ���� �� ���� ��� ����
                    if (ALL_DEVICE_DRIVES_Start_Node == NULL) {

                        ALL_DEVICE_DRIVES_Start_Node = Create_ALL_DEVICE_DRIVES_Node(NULL, Obj_Dir_NAME, NT_NAME, &get_SERIAL_NUM, &result);

                        ALL_DEVICE_DRIVES_Current_Node = ALL_DEVICE_DRIVES_Start_Node;
                    }
                    else {
                        ALL_DEVICE_DRIVES_Current_Node = Append_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Current_Node, Obj_Dir_NAME, NT_NAME, &get_SERIAL_NUM, &result);
                    }
                }
                else {
                    // �̹����� ��, UPDATE��
                    Update_ALL_DEVICE_DRIVES_Node(Node_from_Finder, Obj_Dir_NAME, &result);
                }

            }
            else {
                // �̹����� ��, UPDATE��
                Update_ALL_DEVICE_DRIVES_Node(Node_from_Finder, Obj_Dir_NAME, &result);
            }
            //////////////Print_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node);

        }
        return External_DISK_USB;
    }





    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Drive: %wZ -> %wZ\n", *Obj_Dir_NAME, *NT_PATH);
    return DEVICE_None;
}

BOOLEAN Get_USB_Serial(PUNICODE_STRING INPUT_USB_DIR_INFO, PUNICODE_STRING OUTPUT_USB_SERIAL_NUMBER, ULONG32 Filter_Index, BOOLEAN Filter_with_Ampersand) {
    if (OUTPUT_USB_SERIAL_NUMBER == NULL) return FALSE;

    PWCH filter = L"USB#"; // �׳� �Ϲ� �Լ����� ������ ������Ʈ ���
    PWCH filter2 = L"\\??\\USB#"; // PNP������ ������ �ɺ��� ��ũ
    PWCH filter3 = L"STORAGE#Volume#_??_USBSTOR#"; // USB�� �ý��ۿ� ��ϵ� ���� ( ������ �Ҵ���� )

    // USB#�� �����ϴ��� Ȯ��
    if (

        (memcmp(INPUT_USB_DIR_INFO->Buffer, filter, sizeof(L"USB#") - sizeof(WCHAR)) == 0) || // �׳� �Ϲ� �Լ����� ������ ������Ʈ ����ΰ�?
        (memcmp(INPUT_USB_DIR_INFO->Buffer, filter2, sizeof(L"\\??\\USB#") - sizeof(WCHAR)) == 0) ||// PNP���� ������ �ɺ��� ��ũ�ΰ�?
        (memcmp(INPUT_USB_DIR_INFO->Buffer, filter3, sizeof(L"STORAGE#Volume#_??_USBSTOR#") - sizeof(WCHAR)) == 0)

        ) {

        ULONG32 detect_count = Filter_Index;// 2; // 2��° "#" ã��
        ULONG32 current_detect_count = 0;
        BOOLEAN is_SERIAL_GETTING = FALSE;

        // �ʱ�ȭ
        OUTPUT_USB_SERIAL_NUMBER->Length = 0;
        OUTPUT_USB_SERIAL_NUMBER->MaximumLength = INPUT_USB_DIR_INFO->MaximumLength;
        OUTPUT_USB_SERIAL_NUMBER->Buffer = ExAllocatePoolWithTag(NonPagedPool, OUTPUT_USB_SERIAL_NUMBER->MaximumLength, 'Seri');
        if (OUTPUT_USB_SERIAL_NUMBER->Buffer == NULL) return FALSE;
        memset(OUTPUT_USB_SERIAL_NUMBER->Buffer, 0, OUTPUT_USB_SERIAL_NUMBER->MaximumLength);

        ULONG32 SERIAL_current_offset = 0;

        // USB ���丮 ������ �� ���ھ� Ȯ��
        for (ULONG32 count = 0; count < INPUT_USB_DIR_INFO->Length / sizeof(WCHAR); count++) {
            if (is_SERIAL_GETTING) {
                // �ø��� ��ȣ �� �˻�
                if ((INPUT_USB_DIR_INFO->Buffer[count] == L'#') || (Filter_with_Ampersand && (INPUT_USB_DIR_INFO->Buffer[count] == L'&'))) {
                    is_SERIAL_GETTING = FALSE;

                    OUTPUT_USB_SERIAL_NUMBER->Buffer[SERIAL_current_offset] = L'\0';
                    OUTPUT_USB_SERIAL_NUMBER->Length = (USHORT)(SERIAL_current_offset * sizeof(WCHAR)); // Length�� ���� �� ���� �ؾ���
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " ----SERIAL----- %wZ \n", OUTPUT_USB_SERIAL_NUMBER);

                    return TRUE;
                }

                // �ø��� ��ȣ ����
                if (SERIAL_current_offset < OUTPUT_USB_SERIAL_NUMBER->MaximumLength / sizeof(WCHAR) - 1) {
                    OUTPUT_USB_SERIAL_NUMBER->Buffer[SERIAL_current_offset] = INPUT_USB_DIR_INFO->Buffer[count];
                    SERIAL_current_offset += 1;
                }
                else {
                    break; // ���� �����÷� ����
                }
                continue;
            }

            // "#" ã��
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%wc\n", INPUT_USB_DIR_INFO->Buffer[count]);
            if (INPUT_USB_DIR_INFO->Buffer[count] == L'#') {
                current_detect_count += 1;

                if (current_detect_count == detect_count) {
                    is_SERIAL_GETTING = TRUE;
                }
            }
        }

        // �ø��� ��ȣ�� ã�� ���� ��� �޸� ����
        ExFreePoolWithTag(OUTPUT_USB_SERIAL_NUMBER->Buffer, 'Seri');
        OUTPUT_USB_SERIAL_NUMBER->Buffer = NULL;
    }

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " --- serial FALSE \n");
    return FALSE;
}

BOOLEAN is_check_NetworkDrive(PUNICODE_STRING NT_DEVICE_PATH) {
    if (wcsncmp(NT_DEVICE_PATH->Buffer, L"\\Device\\Mup", sizeof(L"\\Device\\Mup") / sizeof(WCHAR)) == 0) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}