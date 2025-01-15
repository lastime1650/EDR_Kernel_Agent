#ifndef SHA256_H
#define SHA256_H



#include <ntifs.h>
#include <ntddk.h>




#include <ntstrsafe.h>
#include <bcrypt.h>


#define SHA256_String_Byte_Length 65


//해시값 생성
NTSTATUS GET_SHA256_HASHING(PVOID input_FILE_BUFFER, ULONG input_FILE_SIZE, _Out_ CHAR output_HashString[SHA256_String_Byte_Length]);

/*
// 파일경로 -> SHA256
BOOLEAN FILE_TO_SHA256(
    UNICODE_STRING input_file_path,
    CHAR output_HashString[SHA256_String_Byte_Length]
);
*/

#endif