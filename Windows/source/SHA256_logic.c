#include "SHA256.h"

NTSTATUS GET_SHA256_HASHING(PVOID input_FILE_BUFFER, ULONG input_FILE_SIZE, _Out_ CHAR output_HashString[SHA256_String_Byte_Length] ) {

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



// --- 실제 해시 및 파일 처리 함수 ---
NTSTATUS Get_SHA256_with_Big_File(
    PUNICODE_STRING FilePath,
    PLARGE_INTEGER FileSize, // 파일 크기 반환용
    CHAR output_HashString[SHA256_String_Byte_Length]
)
{
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE fileHandle = NULL;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    FILE_STANDARD_INFORMATION fileStdInfo = { 0 };

    BCRYPT_ALG_HANDLE hAlgorithm = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;
    ULONG cbHashObject = 0;
    ULONG cbResult = 0;
    PVOID hashObject = NULL; // 해시 객체 버퍼 (NonPagedPool 권장)

    PVOID readBuffer = NULL;
    // ULONG readBufferSize = 1 * 1024 * 1024; // 1MB 버퍼
    ULONG readBufferSize = 4 * 1024 * 1024; // 4MB 버퍼
    LARGE_INTEGER currentOffset = { 0 };


    // 출력 파라미터 초기화
    FileSize->QuadPart = 0;

    // --- 1. CNG 해시 알고리즘 준비 ---
    status = BCryptOpenAlgorithmProvider(
        &hAlgorithm,
        BCRYPT_SHA256_ALGORITHM,
        NULL,
        0);
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: BCryptOpenAlgorithmProvider failed: 0x%X\n", status);
        goto Cleanup;
    }

    status = BCryptGetProperty(
        hAlgorithm,
        BCRYPT_OBJECT_LENGTH,
        (PUCHAR)&cbHashObject,
        sizeof(ULONG),
        &cbResult,
        0);
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: BCryptGetProperty(OBJECT_LENGTH) failed: 0x%X\n", status);
        goto Cleanup;
    }

    // 해시 객체는 NonPagedPool 이 안전할 수 있음 (IRQL 제약 가능성 고려)
    hashObject = ExAllocatePoolZero(NonPagedPoolNx, cbHashObject, 'hObj');
    if (hashObject == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    status = BCryptCreateHash(
        hAlgorithm,
        &hHash,
        (PUCHAR)hashObject,
        cbHashObject,
        NULL,
        0,
        0);
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: BCryptCreateHash failed: 0x%X\n", status);
        goto Cleanup;
    }

    // --- 2. 파일 열기 (ZwCreateFile 사용) ---
    InitializeObjectAttributes(
        &objectAttributes,
        FilePath,
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, // 커널 핸들 사용
        NULL,
        NULL);

    // ZwCreateFile 사용
    status = ZwCreateFile(
        &fileHandle,
        FILE_GENERIC_READ | SYNCHRONIZE, // 최소한의 권한만 요청
        &objectAttributes,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ, // 공유 허용
        FILE_OPEN, // 이미 존재하는 파일만 열기
        FILE_SYNCHRONOUS_IO_NONALERT, // 동기 I/O
        NULL,
        0
    );
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: ZwCreateFile failed for %wZ: 0x%X\n", FilePath, status);
        fileHandle = NULL; // 실패 시 핸들 무효화
        goto Cleanup;
    }

    // --- 3. 파일 크기 조회 (ZwQueryInformationFile 사용) ---
    status = ZwQueryInformationFile(
        fileHandle,
        &ioStatusBlock,
        &fileStdInfo,
        sizeof(FILE_STANDARD_INFORMATION),
        FileStandardInformation); // 정보 클래스
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: ZwQueryInformationFile failed: 0x%X\n", status);
        goto Cleanup;
    }
    FileSize->QuadPart = fileStdInfo.EndOfFile.QuadPart;

    // 파일 크기가 0이면 처리 생략
    if (FileSize->QuadPart == 0) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: File %wZ is empty. Skipping hash.\n", FilePath);
        status = STATUS_SUCCESS;
        goto Cleanup;
    }

    // --- 4. 읽기 버퍼 할당 ---
    // PASSIVE_LEVEL이므로 PagedPool 사용 가능
    readBuffer = ExAllocatePoolZero(PagedPool, readBufferSize, 'rBuf');
    if (readBuffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    // --- 5. 파일 읽기 및 해시 업데이트 루프 (ZwReadFile 사용) ---
    while (currentOffset.QuadPart < FileSize->QuadPart) {
        ULONG bytesToRead = readBufferSize;
        ULONG bytesRead = 0;

        // 마지막 청크 크기 조절
        LONGLONG remaining = FileSize->QuadPart - currentOffset.QuadPart;
        if ((LONGLONG)bytesToRead > remaining) {
            bytesToRead = (ULONG)remaining;
        }

        if (bytesToRead == 0) {
            // 더 읽을 데이터가 없음 (remaining == 0)
            break;
        }

        // 파일 읽기 (ZwReadFile 사용)
        status = ZwReadFile(
            fileHandle,
            NULL,           // Event (동기 I/O 사용 시 NULL)
            NULL,           // ApcRoutine
            NULL,           // ApcContext
            &ioStatusBlock, // 상태 블록
            readBuffer,     // 읽기 버퍼
            bytesToRead,    // 읽을 바이트 수
            &currentOffset, // 오프셋 지정 (중요!)
            NULL            // Key (사용 안 함)
        );

        if (status == STATUS_PENDING) {
            // 동기 I/O 이므로 KeWaitForSingleObject로 대기
            status = KeWaitForSingleObject(fileHandle, Executive, KernelMode, FALSE, NULL);
            if (NT_SUCCESS(status)) {
                status = ioStatusBlock.Status; // 실제 I/O 결과 상태
            }
        }

        if (!NT_SUCCESS(status)) {
            if (status == STATUS_END_OF_FILE)
                break;
            // STATUS_END_OF_FILE 은 ZwReadFile에 오프셋을 넘겼을 때,
            // 오프셋이 파일 끝이나 그 이후일 경우에만 반환됨.
            // 루프 조건에서 걸러지므로, 여기서 Non-SUCCESS는 실제 I/O 오류임.
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: ZwReadFile failed at offset %lld: 0x%X\n",
                currentOffset.QuadPart, status);
            goto Cleanup;
        }

        // 실제 읽은 바이트 수 확인
        bytesRead = (ULONG)ioStatusBlock.Information;

        // 오프셋을 수동으로 업데이트 해야 함!!!
        currentOffset.QuadPart += bytesRead; // <-- 이 줄이 핵심 수정 사항
        FileSize->QuadPart += bytesRead;

        if (bytesRead > 0) {
            // 읽은 데이터로 해시 업데이트
            status = BCryptHashData(
                hHash,
                (PUCHAR)readBuffer,
                bytesRead,
                0);
            if (!NT_SUCCESS(status)) {
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: BCryptHashData failed: 0x%X\n", status);
                goto Cleanup;
            }
        }

        // bytesRead가 0인 경우는 루프 조건 (remaining == 0) 또는 ZwReadFile 오류에서 걸러짐.
        // 따라서 여기서 추가 0-byte 체크는 불필요함.
        // If we reached EOF, loop condition check will handle break on next iteration.

    } // End of while loop

    // --- 6. 최종 해시 값 얻기 ---
    UCHAR HASH_DATA[32] = { 0, };
    status = BCryptFinishHash(
        hHash,
        HASH_DATA,
        32,
        0);
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: BCryptFinishHash failed: 0x%X\n", status);
    }
    else {
        //해시 값으로 처리
        for (ULONG i = 0; i < 32; i++)
        {
            RtlStringCchPrintfA(&output_HashString[i * 2], 3, "%02x", HASH_DATA[i]);
        }
    }


    goto Cleanup;

Cleanup:
    // --- 7. 모든 리소스 정리 ---
    if (readBuffer) {
        ExFreePoolWithTag(readBuffer, 'rBuf');
    }
    if (fileHandle) {
        ZwClose(fileHandle); // 핸들 닫기
    }
    if (hHash) {
        BCryptDestroyHash(hHash); // 해시 객체 소멸
    }
    if (hashObject) {
        // BCryptDestroyHash 가 내부적으로 관리하므로 불필요할 수 있음
        // ExFreePoolWithTag(hashObject, 'hObj');
    }
    if (hAlgorithm) {
        BCryptCloseAlgorithmProvider(hAlgorithm, 0);
    }

    return status;
}