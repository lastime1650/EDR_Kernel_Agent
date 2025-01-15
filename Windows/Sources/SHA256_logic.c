#include "SHA256.h"

NTSTATUS GET_SHA256_HASHING(PVOID input_FILE_BUFFER, ULONG input_FILE_SIZE, _Out_ CHAR output_HashString[SHA256_String_Byte_Length]) {

    NTSTATUS status = STATUS_SUCCESS;

    BOOLEAN is_complete = FALSE;

    BCRYPT_ALG_HANDLE ALG_HANDLE;

    status = BCryptOpenAlgorithmProvider(&ALG_HANDLE, BCRYPT_SHA256_ALGORITHM, NULL, 0); // SHA256 알고리즘 제공자 핸들 얻기 
    if (status != STATUS_SUCCESS) {
        is_complete = FALSE;
        goto EXIT;
    }
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "BCryptOpenAlgorithmProvider 성공 %p", status);
    BCRYPT_HASH_HANDLE HASH_HANDLE;


    status = BCryptCreateHash(ALG_HANDLE, &HASH_HANDLE, NULL, 0, NULL, 0, 0); // 해싱 제공자 핸들 얻기
    if (status != STATUS_SUCCESS) {
        is_complete = FALSE;
        goto EXIT_Algorithm_Release;
    }
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "BCryptCreateHash 성공 %p", status);

    status = BCryptHashData(HASH_HANDLE, (PUCHAR)input_FILE_BUFFER, input_FILE_SIZE, 0); // 본격 해싱하기 
    if (status != STATUS_SUCCESS) {
        is_complete = FALSE;
        goto EXIT_HASH_DESTROY;
    }
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "BCryptHashData 성공 %p", status);


    UCHAR HASH_DATA[32] = { 0, };
    status = BCryptFinishHash(HASH_HANDLE, HASH_DATA, 32, 0); // 길이가 Provider 에 제공된 hash알고리즘의 길이와 같아야 작동된다. 
    if (status != STATUS_SUCCESS) {
        is_complete = FALSE;
        goto EXIT_HASH_DESTROY;
    }


    //해시 값으로 처리
    for (ULONG i = 0; i < 32; i++)
    {
        RtlStringCchPrintfA(&output_HashString[i * 2], 3, "%02x", HASH_DATA[i]);
    }
    is_complete = TRUE;


    //memcpy(&server_data->SHA256, HashString, 65);
    goto EXIT_HASH_DESTROY;


EXIT_HASH_DESTROY:
    BCryptDestroyHash(HASH_HANDLE);
    goto EXIT_Algorithm_Release;

EXIT_Algorithm_Release:
    BCryptCloseAlgorithmProvider(ALG_HANDLE, 0);
    goto EXIT;

EXIT:
    if (is_complete) {
        return STATUS_SUCCESS;
    }
    else {
        return STATUS_UNSUCCESSFUL;
    }
}



/*
#include "File_io.h"
BOOLEAN FILE_TO_SHA256(
    UNICODE_STRING input_file_path,
    CHAR output_HashString[SHA256_String_Byte_Length]
) {
    K_object_init_check_also_lock_ifyouwant(&file_io__mutex, TRUE);
    BOOLEAN return_bool = TRUE;

    if ( !output_HashString)
        goto EXIT;

    // 1. 파일 버퍼 + 사이즈 추출
    PUCHAR File_Buffer = NULL;
    ULONG File_Size = 0;
    if (ALL_in_ONE_FILE_IO(&File_Buffer, &File_Size, input_file_path, READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH) != STATUS_SUCCESS) {
        return_bool = FALSE;
        goto LABEL_1;
    }

    // 2. SHA256
    if (GET_SHA256_HASHING(File_Buffer, File_Size, output_HashString) != STATUS_SUCCESS) {
        return_bool = FALSE;
    }
    else {
        return_bool = TRUE;
    }
    goto LABEL_1;


LABEL_1:
    {
        if(File_Buffer)
            ExFreePoolWithTag(File_Buffer, 'FILE');

        goto EXIT;
    }
EXIT:
    {
        K_object_lock_Release(&file_io__mutex);
        return return_bool;
    }
}
*/