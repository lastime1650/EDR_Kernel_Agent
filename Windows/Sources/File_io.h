#ifndef File_io_H
#define File_io_H

#include <ntifs.h>
#include <ntddk.h>

typedef enum ALL_in_ONE_FILE_IO_info {
    WRITE_MODE,
    READ_MODE,
    READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH
}ALL_in_ONE_FILE_IO_info, * PALL_in_ONE_FILE_IO_info;

#include "KEVENT_or_KMUTEX_init.h"
extern K_EVENT_or_MUTEX_struct file_io__mutex;

/*파일 핸들 얻기*/
NTSTATUS Create_FILE(PHANDLE File_handle, UNICODE_STRING Create_File_Path, POBJECT_ATTRIBUTES objAttr, PIO_STATUS_BLOCK ioStatusBlock, ACCESS_MASK Desired_access, ULONG ShareOption, ULONG CreateOption);

NTSTATUS Open_FILE(PHANDLE File_handle, UNICODE_STRING Create_File_Path, POBJECT_ATTRIBUTES objAttr, PIO_STATUS_BLOCK ioStatusBlock, ACCESS_MASK Desired_access, ULONG ShareOption);

NTSTATUS Write_FILE(HANDLE File_handle, PVOID INPUT_DATA, ULONG INPUT_DATA_SIZE, PIO_STATUS_BLOCK ioStatusBlock);

NTSTATUS Read_FILE(_In_ HANDLE File_handle, _In_ PVOID OUTPUT_DATA, _In_ ULONG inPUT_DATA_SIZE, PIO_STATUS_BLOCK ioStatusBlock);

// 실 사용 함수
NTSTATUS ALL_in_ONE_FILE_IO(PVOID* RAW_DATA, ULONG* RAW_DATA_SIZE, UNICODE_STRING File_path, ALL_in_ONE_FILE_IO_info custom_MODE);





/*

    유틸

*/

#include "PE_Manager_Struct.h"
typedef struct PE_info {
    PIMAGE_DOS_HEADER_c dosHeader; 
    PIMAGE_NT_HEADERS_c ntHeaders;
    PIMAGE_FILE_HEADER_c fileHeader;
    PIMAGE_OPTIONAL_HEADER32_c optionalHeader32;
    PIMAGE_OPTIONAL_HEADER64_c optionalHeader64;


    ULONG32 PE_Architecture;
    ULONG32 Output_CertificateType;
}PE_info, * PPE_info;


#define SHA256_String_Byte_Length 65
BOOLEAN FILE_to_INFO(
    UNICODE_STRING* opt_file_path,
    HANDLE* opt_PID,
    ULONG32* output_file_size,
    PE_info* output_pe_info,
    CHAR* output_sha256
);

// 파일 길이 가져오기
BOOLEAN get_file_size(
    UNICODE_STRING* opt_file_path,
    HANDLE* opt_PID,

    ULONG32* OUTPUT_knwon_file_size,
    BOOLEAN is_process // 프로세스 파일인가?
);

#endif