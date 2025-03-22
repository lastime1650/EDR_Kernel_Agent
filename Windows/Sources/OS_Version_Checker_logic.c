#include "OS_Version_Check.h"

// ������ 10�� �� ���� ��ȣ�� �����մϴ�.
#define WINDOWS_10_MAJOR_VERSION 10

/*
 * @brief � ü���� ������ Ȯ���ϰ� Windows 10 �̻����� ���θ� ��ȯ�մϴ�.
 *
 * @return Windows 10 �̻��̸� STATUS_SUCCESS��, �׷��� ������ STATUS_NOT_SUPPORTED�� ��ȯ�մϴ�.
 *         RtlGetVersion �Լ� ȣ���� �����ϸ� �ش� ���� ���¸� ��ȯ�մϴ�.
 */
NTSTATUS OS_Version_Checker(PUCHAR* opt_output_os_info) {

    RTL_OSVERSIONINFOW osVersionInfo = { 0 };
    osVersionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

    // RtlGetVersion�� ����Ͽ� OS ���� ������ �����ɴϴ�.
    NTSTATUS status = RtlGetVersion(&osVersionInfo);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    

    // Windows 10 �̻����� Ȯ���մϴ�.
    if (osVersionInfo.dwMajorVersion >= WINDOWS_10_MAJOR_VERSION) {

        if (opt_output_os_info) 
            Output_OS_info_with_alloc(
                opt_output_os_info,
                osVersionInfo.dwMajorVersion,
                osVersionInfo.dwMinorVersion,
                osVersionInfo.dwBuildNumber,
                osVersionInfo.dwOSVersionInfoSize,
                osVersionInfo.dwPlatformId
            );


        return STATUS_SUCCESS;
    }
    else {
        return STATUS_NOT_SUPPORTED;
    }
}

/*
 * @brief � ü���� ���� ������ �����ɴϴ�.
 *
 * @param output � ü�� ���� ������ ������ OSVERSIONINFOW ����ü�� ���� �������Դϴ�.
 *               ȣ���ڴ� �� ����ü�� dwOSVersionInfoSize ����� sizeof(OSVERSIONINFOW)�� �����ؾ� �մϴ�.
 *
 * @return �����ϸ� STATUS_SUCCESS�� ��ȯ�մϴ�.
 *         output�� NULL�̸� STATUS_INVALID_PARAMETER�� ��ȯ�մϴ�.
 *         RtlGetVersion �Լ� ȣ���� �����ϸ� �ش� ���� ���¸� ��ȯ�մϴ�.
 */
NTSTATUS Get_OS_Versions(OSVERSIONINFOW* output) {
    if (output == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    // RtlGetVersion�� ����Ͽ� OS ���� ������ �����ɴϴ�.
    output->dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
    NTSTATUS status = RtlGetVersion(output);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    return STATUS_SUCCESS;
}

#pragma warning(disable:4996)
#include <ntstrsafe.h>
#define alloc_tag 'OSin'
NTSTATUS Output_OS_info_with_alloc(PUCHAR* output_ansi, ULONG MajorVersion, ULONG MinorVersion, ULONG BuildVersion, ULONG OSVersionInfoSize, ULONG PlatformId) {
    if (!output_ansi)
        return STATUS_INVALID_PARAMETER_1;


    CHAR arch[3] = { 0, };

#ifdef _M_AMD64
    RtlCopyMemory(arch, "64", sizeof(arch));
#elif defined(_M_X64)
    RtlCopyMemory(arch, "64", sizeof(arch));
#elif defined(_WIN64)
    RtlCopyMemory(arch, "64", sizeof(arch));
#elif defined(_WIN32)
    RtlCopyMemory(arch, "32", sizeof(arch));
#elif defined(_M_IX86)
    RtlCopyMemory(arch, "32", sizeof(arch));
#else
    return STATUS_NOT_SUPPORTED;
#endif


    *output_ansi = ExAllocatePoolWithTag(NonPagedPool, 256, alloc_tag);
    if (!*output_ansi)
        return STATUS_MEMORY_NOT_ALLOCATED;

    memset(*output_ansi, 0, 128);

    NTSTATUS status = STATUS_UNSUCCESSFUL;

    status = RtlStringCbPrintfA(
        (NTSTRSAFE_PSTR)*output_ansi,
        256, // �Ҵ�� ���� ũ�� (����Ʈ)
        "Windows(x%s) %lu.%lu.%lu.%lu.%lu",
        arch,
        MajorVersion,
        MinorVersion,
        BuildVersion,
        OSVersionInfoSize,
        PlatformId
    );

    

    

    return status;
}
VOID OS_info_free(PUCHAR _ansi) {
    if (_ansi)
        ExFreePoolWithTag(_ansi, alloc_tag);
}