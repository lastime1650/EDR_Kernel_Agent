#ifndef PE_Manager_H
#define PE_Manager_H

#include <ntifs.h>

#include "DynamicData_Linked_list.h"
#include "PE_Manager_Struct.h"


NTSTATUS ExtractPEHeaders(
    PUCHAR baseAddress,
    PIMAGE_DOS_HEADER_c* dosHeader,
    PIMAGE_NT_HEADERS_c* ntHeaders,
    PIMAGE_FILE_HEADER_c* fileHeader,
    _Outptr_opt_ PIMAGE_OPTIONAL_HEADER32_c* optionalHeader32,
    _Outptr_opt_ PIMAGE_OPTIONAL_HEADER64_c* optionalHeader64,
    ULONG32* PE_Architecture,
    _Outptr_opt_ PDynamicData* Output_Optional_Directories
);

BOOLEAN Release_IMAGE_DATA_DIRECTORY_c(PDynamicData StartAddr);


VOID Check_Digital_Certificate_info(PUCHAR BaseAddress, PDynamicData Input_Optional_Directories, ULONG32* Output_CertificateType);

#endif