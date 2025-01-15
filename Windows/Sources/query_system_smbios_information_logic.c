#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "query_system_smbios_information.h"

#pragma warning(disable:4101)

/*
    <이 코드의 장점>
    AGENT가 설치되는 HOST 고유의 하드웨어 정보로 AGENT_ID의 고유성을 보장한다.

    <어떻게 구현?>
        (1) 일단, 파싱을 해서 특정부분을 뽑아서 처리하거나, 
        (2) 또는 한꺼번에 하는 방식이 있다.


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
    // ACPI 테이블을 읽는 예제 코드 (매우 간단히 설명)
    NTSTATUS status;
    ULONG tableSize;
    SYSTEM_FIRMWARE_TABLE_INFORMATION* firmwareTableInfo;
    ULONG bufferSize = sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION) + 1024;

    firmwareTableInfo = (SYSTEM_FIRMWARE_TABLE_INFORMATION*)ExAllocatePoolWithTag(PagedPool, bufferSize, SMBIOS_TAG); //먼저 어느정도 길이를 가져와야하는 것으로 확인.(쿼리 전, Hint정보를 필드에 넣고 쿼리하기 때문) 
    if (!firmwareTableInfo) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "메모리 공간 부족\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    RtlZeroMemory(firmwareTableInfo, bufferSize);
    firmwareTableInfo->ProviderSignature = SMBIOS_TAG; // ACPI 시그니처 설정
    firmwareTableInfo->Action = SystemFirmwareTable_Get; // " Enum " 으로 펌웨어 정보 가져오도록함 


    
    while (( status = ZwQuerySystemInformation(SystemFirmwareTableInformation, firmwareTableInfo, bufferSize, &tableSize) )  != STATUS_SUCCESS) {
        
        // 선 할당 해제
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
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SystemFirmwareTableInformation 쿼리 결과 -> STATUS_BUFFER_TOO_SMALL 이기에, 재할당했지만 메모리 공간 부족;\n");
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlZeroMemory(firmwareTableInfo, tableSize);
            firmwareTableInfo->ProviderSignature = SMBIOS_TAG;
            firmwareTableInfo->Action = SystemFirmwareTable_Get;
            bufferSize = tableSize;  // bufferSize를 tableSize로 업데이트
        }
        else {
            //ExFreePoolWithTag(firmwareTableInfo, 'RSMB');
            return STATUS_UNSUCCESSFUL;
        }

        continue;
    }

    
    //쿼리 성공후 처리를 구현하라
    
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SystemFirmwareTableInformation 쿼리 성공!\n");




   // 구조체는 SMBIOS_type_structs.h 에 구현되어 있다.



    // SM-BIOS 출력 -  파싱
    ULONG offset = 0;
    ULONG remember_start_offset = 0;
    while (offset < firmwareTableInfo->TableBufferLength) {


        PSMBIOS_STRUCTURE header = (PSMBIOS_STRUCTURE)(firmwareTableInfo->TableBuffer + offset);
        /*
            펌웨어 파싱 법.

            1. 먼저 총 길이를 구한다.
            2. 그 다음 위치까지 데이터를 뽑도록 한다
        
        */



        remember_start_offset = offset; // 현재 위치 기억



        offset += header->Length; // (오프셋 갱신됨) ( 해당 SMBIOS 타입의 헤더 총길이 . 이 길이를 넘어서면, 실제 데이터이다. <헤더는> Index를 나타낸다.  ) 





        if (header->Length == 0) {
            break;  // 무한 루프 방지
        }

        // 문자열 섹션 스킵 ( 다음 타입으로 이동하기 위한 작업 ) 
        while (offset < firmwareTableInfo->TableBufferLength && *(USHORT*)(firmwareTableInfo->TableBuffer + offset) != 0) {
            offset++; // 오프셋 갱신
        }
        offset += 2; // NULL 문자 2개 스킵 ( 기본 처리임 ) (오프셋 갱신)

        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "오프셋 전/후 [전]: %d , [후]: %d \n\n", remember_start_offset,offset );
        


        // 뽑아낼 SMBIOS 타입을 가져와, 동적 전역변수에 축적함. -> 그리고 SHA512 처리. 
        switch (header->Type) {
        case 1:
            /*
                Type 1: System Information

                    시스템 제조사, 제품명, 버전, 시리얼 번호 등 시스템의 고유 정보를 포함합니다.
                    시스템 UUID (Universally Unique Identifier)는 재부팅해도 변하지 않습니다.

            */
            // ( header + header->Length ) // RAW_DATA시작주소

            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "memcpy( %p , %p , %d )\n", Driver_ID.SMBIOS_TYPE_1, (PUCHAR)((PUCHAR)header + (ULONG32)header->Length), (offset - remember_start_offset) - header->Length );
            
            if (SMBIOS_1_Type_DATA) {
                
                *SMBIOS_1_Type_DATA = ExAllocatePoolWithTag(PagedPool, (offset - remember_start_offset) - header->Length, SMBIOS_TAG);
                memset(*SMBIOS_1_Type_DATA, 0, ((offset - remember_start_offset) - header->Length));
                if (*SMBIOS_1_Type_DATA) {
                    memcpy(*SMBIOS_1_Type_DATA, (PUCHAR)((PUCHAR)header + (ULONG32)header->Length), (offset - remember_start_offset) - header->Length); // 하드웨어 타입 1 정보 저장
                    *SMBIOS_1_Type_DATA_SIZE = (offset - remember_start_offset) - header->Length;
                }
                

            }
            
            //PrintBuffer((PUCHAR)((PUCHAR)header + (ULONG32)header->Length), (offset - remember_start_offset) - header->Length);

            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "구한 PSMBIOSStruct 주소 -> %p, 타입-> %d, 길이-> %d\n\n", header, header->Type, header->Length);
            break;
        case 2:
            /*
                Type 2: Baseboard (or Module) Information

                    메인보드의 제조사, 제품명, 버전, 시리얼 번호 등 메인보드의 고유 정보를 포함합니다.
                    메인보드 시리얼 번호는 재부팅해도 변하지 않습니다.
            */

            if (SMBIOS_2_Type_DATA) {
                *SMBIOS_2_Type_DATA = ExAllocatePoolWithTag(PagedPool, (offset - remember_start_offset) - header->Length, SMBIOS_TAG);
                memset(*SMBIOS_2_Type_DATA, 0, ((offset - remember_start_offset) - header->Length));
                if (*SMBIOS_2_Type_DATA) {
                    memcpy(*SMBIOS_2_Type_DATA, (PUCHAR)((PUCHAR)header + (ULONG32)header->Length), ((offset - header->Length) - remember_start_offset)); // 하드웨어 타입 2 정보 저장
                    *SMBIOS_2_Type_DATA_SIZE = (offset - remember_start_offset) - header->Length;
                }
            }

            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "memcpy( %p , %p , %d )\n", Driver_ID.SMBIOS_TYPE_2, (PUCHAR)((PUCHAR)header + (ULONG32)header->Length), (offset - remember_start_offset) - header->Length);
            

            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "구한 PSMBIOSStruct 주소 -> %p, 타입-> %d, 길이-> %d\n", header, header->Type, header->Length);
            break;
        default:
            /*
                나머지 타입은 스킵
            */
            break;
        }

        remember_start_offset = offset; // 갱신된 오프셋의 정보를 넣어 업데이트

        

        if (offset > firmwareTableInfo->TableBufferLength) {
            break;  // 오프셋이 동적할당된 최대 가능한 주소 범위를 초과하면 종료 ( 혹시 모를 안전장치 ) 
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