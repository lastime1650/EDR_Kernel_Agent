#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "query_system_smbios_information.h"

#pragma warning(disable:4101)

/*
    <�� �ڵ��� ����>
    AGENT�� ��ġ�Ǵ� HOST ������ �ϵ���� ������ AGENT_ID�� �������� �����Ѵ�.

    <��� ����?>
        (1) �ϴ�, �Ľ��� �ؼ� Ư���κ��� �̾Ƽ� ó���ϰų�, 
        (2) �Ǵ� �Ѳ����� �ϴ� ����� �ִ�.


*/

void PrintBuffer(PUCHAR buffer, SIZE_T maxLength) {
    SIZE_T i;
    for (i = 0; i < maxLength; i++) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%02X ", buffer[i]);
    }
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n");
}

NTSTATUS Query_SMBIOS_information(
    PUCHAR* SMBIOS_1_Type_DATA, ULONG32* SMBIOS_1_Type_DATA_SIZE,
    PUCHAR* SMBIOS_2_Type_DATA, ULONG32* SMBIOS_2_Type_DATA_SIZE
) {
    // ACPI ���̺��� �д� ���� �ڵ� (�ſ� ������ ����)
    NTSTATUS status;
    ULONG tableSize;
    SYSTEM_FIRMWARE_TABLE_INFORMATION* firmwareTableInfo;
    ULONG bufferSize = sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION) + 1024;

    firmwareTableInfo = (SYSTEM_FIRMWARE_TABLE_INFORMATION*)ExAllocatePoolWithTag(PagedPool, bufferSize, SMBIOS_TAG); //���� ������� ���̸� �����;��ϴ� ������ Ȯ��.(���� ��, Hint������ �ʵ忡 �ְ� �����ϱ� ����) 
    if (!firmwareTableInfo) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�޸� ���� ����\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    RtlZeroMemory(firmwareTableInfo, bufferSize);
    firmwareTableInfo->ProviderSignature = SMBIOS_TAG; // ACPI �ñ״�ó ����
    firmwareTableInfo->Action = SystemFirmwareTable_Get; // " Enum " ���� �߿��� ���� ������������ 


    
    while (( status = ZwQuerySystemInformation(SystemFirmwareTableInformation, firmwareTableInfo, bufferSize, &tableSize) )  != STATUS_SUCCESS) {
        
        // �� �Ҵ� ����
        ExFreePoolWithTag(firmwareTableInfo, SMBIOS_TAG);
        firmwareTableInfo = NULL;

        if (
            (status == STATUS_INVALID_INFO_CLASS) ||
            (status == STATUS_INVALID_DEVICE_REQUEST) ||
            (status == STATUS_NOT_IMPLEMENTED) ||
            (tableSize == 0)
            ) {
            //ExFreePoolWithTag(firmwareTableInfo, 'RSMB');
            return STATUS_UNSUCCESSFUL;
        }
        else if (status == STATUS_BUFFER_TOO_SMALL) {
            //ExFreePoolWithTag(firmwareTableInfo, 'RSMB');
            firmwareTableInfo = (SYSTEM_FIRMWARE_TABLE_INFORMATION*)ExAllocatePoolWithTag(PagedPool, tableSize, SMBIOS_TAG);
            if (!firmwareTableInfo) {
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SystemFirmwareTableInformation ���� ��� -> STATUS_BUFFER_TOO_SMALL �̱⿡, ���Ҵ������� �޸� ���� ����;\n");
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlZeroMemory(firmwareTableInfo, tableSize);
            firmwareTableInfo->ProviderSignature = SMBIOS_TAG;
            firmwareTableInfo->Action = SystemFirmwareTable_Get;
            bufferSize = tableSize;  // bufferSize�� tableSize�� ������Ʈ
        }
        else {
            //ExFreePoolWithTag(firmwareTableInfo, 'RSMB');
            return STATUS_UNSUCCESSFUL;
        }

        continue;
    }

    
    //���� ������ ó���� �����϶�
    
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SystemFirmwareTableInformation ���� ����!\n");




   // ����ü�� SMBIOS_type_structs.h �� �����Ǿ� �ִ�.



    // SM-BIOS ��� -  �Ľ�
    ULONG offset = 0;
    ULONG remember_start_offset = 0;
    while (offset < firmwareTableInfo->TableBufferLength) {


        PSMBIOS_STRUCTURE header = (PSMBIOS_STRUCTURE)(firmwareTableInfo->TableBuffer + offset);
        /*
            �߿��� �Ľ� ��.

            1. ���� �� ���̸� ���Ѵ�.
            2. �� ���� ��ġ���� �����͸� �̵��� �Ѵ�
        
        */



        remember_start_offset = offset; // ���� ��ġ ���



        offset += header->Length; // (������ ���ŵ�) ( �ش� SMBIOS Ÿ���� ��� �ѱ��� . �� ���̸� �Ѿ��, ���� �������̴�. <�����> Index�� ��Ÿ����.  ) 





        if (header->Length == 0) {
            break;  // ���� ���� ����
        }

        // ���ڿ� ���� ��ŵ ( ���� Ÿ������ �̵��ϱ� ���� �۾� ) 
        while (offset < firmwareTableInfo->TableBufferLength && *(USHORT*)(firmwareTableInfo->TableBuffer + offset) != 0) {
            offset++; // ������ ����
        }
        offset += 2; // NULL ���� 2�� ��ŵ ( �⺻ ó���� ) (������ ����)

        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "������ ��/�� [��]: %d , [��]: %d \n\n", remember_start_offset,offset );
        


        // �̾Ƴ� SMBIOS Ÿ���� ������, ���� ���������� ������. -> �׸��� SHA512 ó��. 
        switch (header->Type) {
        case 1:
            /*
                Type 1: System Information

                    �ý��� ������, ��ǰ��, ����, �ø��� ��ȣ �� �ý����� ���� ������ �����մϴ�.
                    �ý��� UUID (Universally Unique Identifier)�� ������ص� ������ �ʽ��ϴ�.

            */
            // ( header + header->Length ) // RAW_DATA�����ּ�

            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "memcpy( %p , %p , %d )\n", Driver_ID.SMBIOS_TYPE_1, (PUCHAR)((PUCHAR)header + (ULONG32)header->Length), (offset - remember_start_offset) - header->Length );
            
            if (SMBIOS_1_Type_DATA) {
                
                *SMBIOS_1_Type_DATA = ExAllocatePoolWithTag(PagedPool, (offset - remember_start_offset) - header->Length, SMBIOS_TAG);
                memset(*SMBIOS_1_Type_DATA, 0, ((offset - remember_start_offset) - header->Length));
                if (*SMBIOS_1_Type_DATA) {
                    memcpy(*SMBIOS_1_Type_DATA, (PUCHAR)((PUCHAR)header + (ULONG32)header->Length), (offset - remember_start_offset) - header->Length); // �ϵ���� Ÿ�� 1 ���� ����
                    *SMBIOS_1_Type_DATA_SIZE = (offset - remember_start_offset) - header->Length;
                }
                

            }
            
            //PrintBuffer((PUCHAR)((PUCHAR)header + (ULONG32)header->Length), (offset - remember_start_offset) - header->Length);

            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���� PSMBIOSStruct �ּ� -> %p, Ÿ��-> %d, ����-> %d\n\n", header, header->Type, header->Length);
            break;
        case 2:
            /*
                Type 2: Baseboard (or Module) Information

                    ���κ����� ������, ��ǰ��, ����, �ø��� ��ȣ �� ���κ����� ���� ������ �����մϴ�.
                    ���κ��� �ø��� ��ȣ�� ������ص� ������ �ʽ��ϴ�.
            */

            if (SMBIOS_2_Type_DATA) {
                *SMBIOS_2_Type_DATA = ExAllocatePoolWithTag(PagedPool, (offset - remember_start_offset) - header->Length, SMBIOS_TAG);
                memset(*SMBIOS_2_Type_DATA, 0, ((offset - remember_start_offset) - header->Length));
                if (*SMBIOS_2_Type_DATA) {
                    memcpy(*SMBIOS_2_Type_DATA, (PUCHAR)((PUCHAR)header + (ULONG32)header->Length), ((offset - header->Length) - remember_start_offset)); // �ϵ���� Ÿ�� 2 ���� ����
                    *SMBIOS_2_Type_DATA_SIZE = (offset - remember_start_offset) - header->Length;
                }
            }

            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "memcpy( %p , %p , %d )\n", Driver_ID.SMBIOS_TYPE_2, (PUCHAR)((PUCHAR)header + (ULONG32)header->Length), (offset - remember_start_offset) - header->Length);
            

            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���� PSMBIOSStruct �ּ� -> %p, Ÿ��-> %d, ����-> %d\n", header, header->Type, header->Length);
            break;
        default:
            /*
                ������ Ÿ���� ��ŵ
            */
            break;
        }

        remember_start_offset = offset; // ���ŵ� �������� ������ �־� ������Ʈ

        

        if (offset > firmwareTableInfo->TableBufferLength) {
            break;  // �������� �����Ҵ�� �ִ� ������ �ּ� ������ �ʰ��ϸ� ���� ( Ȥ�� �� ������ġ ) 
        }
    }


    
    ExFreePoolWithTag(firmwareTableInfo, SMBIOS_TAG);
    
    return STATUS_SUCCESS;
}


BOOLEAN Release_Query_SMBIOS_information(
    PUCHAR* SMBIOS_1_Type_DATA,
    PUCHAR* SMBIOS_2_Type_DATA
) {
    if (SMBIOS_1_Type_DATA) {
        ExFreePoolWithTag(*SMBIOS_1_Type_DATA, SMBIOS_TAG);
        *SMBIOS_1_Type_DATA = NULL;
    }
    if (SMBIOS_2_Type_DATA) {
        ExFreePoolWithTag(*SMBIOS_2_Type_DATA, SMBIOS_TAG);
        *SMBIOS_2_Type_DATA = NULL;
    }
    return TRUE;
}