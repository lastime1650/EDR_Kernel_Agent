#include "PE_Manager.h"

NTSTATUS ExtractPEHeaders(
    PUCHAR baseAddress,
    PIMAGE_DOS_HEADER_c* dosHeader,
    PIMAGE_NT_HEADERS_c* ntHeaders,
    PIMAGE_FILE_HEADER_c* fileHeader,
    _Outptr_opt_ PIMAGE_OPTIONAL_HEADER32_c* optionalHeader32,
    _Outptr_opt_ PIMAGE_OPTIONAL_HEADER64_c* optionalHeader64,
    ULONG32* PE_Architecture,
    _Outptr_opt_ PDynamicData* Output_Optional_Directories
) {
    if (
        dosHeader == NULL ||
        ntHeaders == NULL ||
        fileHeader == NULL ||
        optionalHeader32 == NULL ||
        optionalHeader64 == NULL ||
        PE_Architecture == NULL ||
		Output_Optional_Directories == NULL
        ) {
		return STATUS_INVALID_PARAMETER;
    }

    PIMAGE_DOS_HEADER_c pDosHeader;
    PIMAGE_NT_HEADERS_c pNtHeaders;


    // 파일 추출


    // 1. DOS Header 추출
    pDosHeader = (PIMAGE_DOS_HEADER_c)baseAddress;

    // DOS 헤더의 e_magic 값 체크 (MZ 시그니처)
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    // 2. e_lfanew 값을 통해 NT 헤더로 이동
    pNtHeaders = (PIMAGE_NT_HEADERS_c)(baseAddress + pDosHeader->e_lfanew);

    // NT 헤더의 Signature 체크 (PE 시그니처)
    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    // 3. FileHeader 추출
    PIMAGE_FILE_HEADER_c pFileHeader = &(pNtHeaders->FileHeader);

    // 4. OptionalHeader 추출
    PDynamicData Start_Address = NULL;
    PDynamicData Current_Address = NULL;

    PIMAGE_OPTIONAL_HEADER32_c pOptionalHeader = (PIMAGE_OPTIONAL_HEADER32_c) & (pNtHeaders->OptionalHeader);
	if (pOptionalHeader->Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        *PE_Architecture = 64;
        *optionalHeader64 = (PIMAGE_OPTIONAL_HEADER64_c) & (pNtHeaders->OptionalHeader);
		*optionalHeader32 = NULL;

		// OptionalHeader의 DataDirectory를 동적할당 [0 ~ 15]
		for (ULONG32 i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
            if (!Start_Address) {
                Start_Address = CreateDynamicData((PUCHAR)&(*optionalHeader64)->DataDirectory[i], sizeof((*optionalHeader64)->DataDirectory[i]));
				Current_Address = Start_Address;
            }
            else {
                PDynamicData tmp = NULL;
                tmp = AppendDynamicData(Current_Address, (PUCHAR)& (*optionalHeader64)->DataDirectory[i], sizeof((*optionalHeader64)->DataDirectory[i]));
                
                if (tmp != NULL) {
                    Current_Address = tmp;
                }
            }
		}
		*Output_Optional_Directories = Start_Address;

	}
	else if (pOptionalHeader->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
		*PE_Architecture = 32;
        *optionalHeader64 = NULL;
        *optionalHeader32 = (PIMAGE_OPTIONAL_HEADER32_c) & (pNtHeaders->OptionalHeader);

        // OptionalHeader의 DataDirectory를 동적할당 [0 ~ 15]
        for (ULONG32 i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
            if (!Start_Address) {
                Start_Address = CreateDynamicData((PUCHAR) & (*optionalHeader32)->DataDirectory[i], sizeof((*optionalHeader32)->DataDirectory[i]));
                Current_Address = Start_Address;
            }
            else {
                PDynamicData tmp = NULL;
                tmp = AppendDynamicData(Current_Address, (PUCHAR) & (*optionalHeader32)->DataDirectory[i], sizeof((*optionalHeader32)->DataDirectory[i]));
                if (tmp != NULL) {
                    Current_Address = tmp;
                }
            }
        }
        *Output_Optional_Directories = Start_Address;
	}
	else {
        *PE_Architecture = 0;
        *optionalHeader64 = NULL;
        *optionalHeader32 = NULL;
	}
   

    // DOS와 NT 헤더를 호출한 함수에 전달
    *dosHeader = pDosHeader;
    *ntHeaders = pNtHeaders;
	*fileHeader = pFileHeader;
	

    return STATUS_SUCCESS;
}



BOOLEAN Release_IMAGE_DATA_DIRECTORY_c(PDynamicData StartAddr) {
    if (!StartAddr) return FALSE;

	RemoveALLDynamicData(StartAddr);

    return TRUE;
}


VOID Check_Digital_Certificate_info(PUCHAR BaseAddress, PDynamicData Input_Optional_Directories, ULONG32* Output_CertificateType) {
    PWIN_cert info = NULL;
    
	PDynamicData Current_Address = Input_Optional_Directories;

    ULONG32 index = 0;
    while (Current_Address != NULL) {

        if (index == IMAGE_DIRECTORY_ENTRY_SECURITY) {
            if (((PIMAGE_DATA_DIRECTORY_c)Current_Address->Data)->Size != 0) {

				BaseAddress += ((PIMAGE_DATA_DIRECTORY_c)Current_Address->Data)->VirtualAddress;
				info = (PWIN_cert)BaseAddress;
                *Output_CertificateType = info->CertificateType;
                return;
            }
            else {
                *Output_CertificateType = 0;
                return;
            }
        }

        Current_Address = (PDynamicData)Current_Address->Next_Addr;
        index++;
    }
}