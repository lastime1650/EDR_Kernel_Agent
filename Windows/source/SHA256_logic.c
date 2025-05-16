#include "SHA256.h"

NTSTATUS GET_SHA256_HASHING(PVOID input_FILE_BUFFER, ULONG input_FILE_SIZE, _Out_ CHAR output_HashString[SHA256_String_Byte_Length] ) {

    NTSTATUS status = STATUS_SUCCESS;

    BOOLEAN is_complete = FALSE;

    BCRYPT_ALG_HANDLE ALG_HANDLE;

    status = BCryptOpenAlgorithmProvider(&ALG_HANDLE, BCRYPT_SHA256_ALGORITHM, NULL, 0); // SHA256 �˰��� ������ �ڵ� ��� 
    if (status != STATUS_SUCCESS) {
        is_complete = FALSE;
        goto EXIT;
    }
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "BCryptOpenAlgorithmProvider ���� %p", status);
    BCRYPT_HASH_HANDLE HASH_HANDLE;


    status = BCryptCreateHash(ALG_HANDLE, &HASH_HANDLE, NULL, 0, NULL, 0, 0); // �ؽ� ������ �ڵ� ���
    if (status != STATUS_SUCCESS) {
        is_complete = FALSE;
        goto EXIT_Algorithm_Release;
    }
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "BCryptCreateHash ���� %p", status);

    status = BCryptHashData(HASH_HANDLE, (PUCHAR)input_FILE_BUFFER, input_FILE_SIZE, 0); // ���� �ؽ��ϱ� 
    if (status != STATUS_SUCCESS) {
        is_complete = FALSE;
        goto EXIT_HASH_DESTROY;
    }
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "BCryptHashData ���� %p", status);


    UCHAR HASH_DATA[32] = { 0, };
    status = BCryptFinishHash(HASH_HANDLE, HASH_DATA, 32, 0); // ���̰� Provider �� ������ hash�˰����� ���̿� ���ƾ� �۵��ȴ�. 
    if (status != STATUS_SUCCESS) {
        is_complete = FALSE;
        goto EXIT_HASH_DESTROY;
    }


    //�ؽ� ������ ó��
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



// --- ���� �ؽ� �� ���� ó�� �Լ� ---
NTSTATUS Get_SHA256_with_Big_File(
    PUNICODE_STRING FilePath,
    PLARGE_INTEGER FileSize, // ���� ũ�� ��ȯ��
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
    PVOID hashObject = NULL; // �ؽ� ��ü ���� (NonPagedPool ����)

    PVOID readBuffer = NULL;
    // ULONG readBufferSize = 1 * 1024 * 1024; // 1MB ����
    ULONG readBufferSize = 4 * 1024 * 1024; // 4MB ����
    LARGE_INTEGER currentOffset = { 0 };


    // ��� �Ķ���� �ʱ�ȭ
    FileSize->QuadPart = 0;

    // --- 1. CNG �ؽ� �˰��� �غ� ---
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

    // �ؽ� ��ü�� NonPagedPool �� ������ �� ���� (IRQL ���� ���ɼ� ���)
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

    // --- 2. ���� ���� (ZwCreateFile ���) ---
    InitializeObjectAttributes(
        &objectAttributes,
        FilePath,
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, // Ŀ�� �ڵ� ���
        NULL,
        NULL);

    // ZwCreateFile ���
    status = ZwCreateFile(
        &fileHandle,
        FILE_GENERIC_READ | SYNCHRONIZE, // �ּ����� ���Ѹ� ��û
        &objectAttributes,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ, // ���� ���
        FILE_OPEN, // �̹� �����ϴ� ���ϸ� ����
        FILE_SYNCHRONOUS_IO_NONALERT, // ���� I/O
        NULL,
        0
    );
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: ZwCreateFile failed for %wZ: 0x%X\n", FilePath, status);
        fileHandle = NULL; // ���� �� �ڵ� ��ȿȭ
        goto Cleanup;
    }

    // --- 3. ���� ũ�� ��ȸ (ZwQueryInformationFile ���) ---
    status = ZwQueryInformationFile(
        fileHandle,
        &ioStatusBlock,
        &fileStdInfo,
        sizeof(FILE_STANDARD_INFORMATION),
        FileStandardInformation); // ���� Ŭ����
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: ZwQueryInformationFile failed: 0x%X\n", status);
        goto Cleanup;
    }
    FileSize->QuadPart = fileStdInfo.EndOfFile.QuadPart;

    // ���� ũ�Ⱑ 0�̸� ó�� ����
    if (FileSize->QuadPart == 0) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: File %wZ is empty. Skipping hash.\n", FilePath);
        status = STATUS_SUCCESS;
        goto Cleanup;
    }

    // --- 4. �б� ���� �Ҵ� ---
    // PASSIVE_LEVEL�̹Ƿ� PagedPool ��� ����
    readBuffer = ExAllocatePoolZero(PagedPool, readBufferSize, 'rBuf');
    if (readBuffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    // --- 5. ���� �б� �� �ؽ� ������Ʈ ���� (ZwReadFile ���) ---
    while (currentOffset.QuadPart < FileSize->QuadPart) {
        ULONG bytesToRead = readBufferSize;
        ULONG bytesRead = 0;

        // ������ ûũ ũ�� ����
        LONGLONG remaining = FileSize->QuadPart - currentOffset.QuadPart;
        if ((LONGLONG)bytesToRead > remaining) {
            bytesToRead = (ULONG)remaining;
        }

        if (bytesToRead == 0) {
            // �� ���� �����Ͱ� ���� (remaining == 0)
            break;
        }

        // ���� �б� (ZwReadFile ���)
        status = ZwReadFile(
            fileHandle,
            NULL,           // Event (���� I/O ��� �� NULL)
            NULL,           // ApcRoutine
            NULL,           // ApcContext
            &ioStatusBlock, // ���� ���
            readBuffer,     // �б� ����
            bytesToRead,    // ���� ����Ʈ ��
            &currentOffset, // ������ ���� (�߿�!)
            NULL            // Key (��� �� ��)
        );

        if (status == STATUS_PENDING) {
            // ���� I/O �̹Ƿ� KeWaitForSingleObject�� ���
            status = KeWaitForSingleObject(fileHandle, Executive, KernelMode, FALSE, NULL);
            if (NT_SUCCESS(status)) {
                status = ioStatusBlock.Status; // ���� I/O ��� ����
            }
        }

        if (!NT_SUCCESS(status)) {
            if (status == STATUS_END_OF_FILE)
                break;
            // STATUS_END_OF_FILE �� ZwReadFile�� �������� �Ѱ��� ��,
            // �������� ���� ���̳� �� ������ ��쿡�� ��ȯ��.
            // ���� ���ǿ��� �ɷ����Ƿ�, ���⼭ Non-SUCCESS�� ���� I/O ������.
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KernelHash: ZwReadFile failed at offset %lld: 0x%X\n",
                currentOffset.QuadPart, status);
            goto Cleanup;
        }

        // ���� ���� ����Ʈ �� Ȯ��
        bytesRead = (ULONG)ioStatusBlock.Information;

        // �������� �������� ������Ʈ �ؾ� ��!!!
        currentOffset.QuadPart += bytesRead; // <-- �� ���� �ٽ� ���� ����
        FileSize->QuadPart += bytesRead;

        if (bytesRead > 0) {
            // ���� �����ͷ� �ؽ� ������Ʈ
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

        // bytesRead�� 0�� ���� ���� ���� (remaining == 0) �Ǵ� ZwReadFile �������� �ɷ���.
        // ���� ���⼭ �߰� 0-byte üũ�� ���ʿ���.
        // If we reached EOF, loop condition check will handle break on next iteration.

    } // End of while loop

    // --- 6. ���� �ؽ� �� ��� ---
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
        //�ؽ� ������ ó��
        for (ULONG i = 0; i < 32; i++)
        {
            RtlStringCchPrintfA(&output_HashString[i * 2], 3, "%02x", HASH_DATA[i]);
        }
    }


    goto Cleanup;

Cleanup:
    // --- 7. ��� ���ҽ� ���� ---
    if (readBuffer) {
        ExFreePoolWithTag(readBuffer, 'rBuf');
    }
    if (fileHandle) {
        ZwClose(fileHandle); // �ڵ� �ݱ�
    }
    if (hHash) {
        BCryptDestroyHash(hHash); // �ؽ� ��ü �Ҹ�
    }
    if (hashObject) {
        // BCryptDestroyHash �� ���������� �����ϹǷ� ���ʿ��� �� ����
        // ExFreePoolWithTag(hashObject, 'hObj');
    }
    if (hAlgorithm) {
        BCryptCloseAlgorithmProvider(hAlgorithm, 0);
    }

    return status;
}