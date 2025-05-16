#include "OS_Version_Check.h"

// 윈도우 10의 주 버전 번호를 정의합니다.
#define WINDOWS_10_MAJOR_VERSION 10

/*
 * @brief 운영 체제의 버전을 확인하고 Windows 10 이상인지 여부를 반환합니다.
 *
 * @return Windows 10 이상이면 STATUS_SUCCESS를, 그렇지 않으면 STATUS_NOT_SUPPORTED를 반환합니다.
 *         RtlGetVersion 함수 호출이 실패하면 해당 실패 상태를 반환합니다.
 */
NTSTATUS OS_Version_Checker(PUCHAR* opt_output_os_info) {

    RTL_OSVERSIONINFOW osVersionInfo = { 0 };
    osVersionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

    // RtlGetVersion을 사용하여 OS 버전 정보를 가져옵니다.
    NTSTATUS status = RtlGetVersion(&osVersionInfo);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    

    // Windows 10 이상인지 확인합니다.
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
 * @brief 운영 체제의 버전 정보를 가져옵니다.
 *
 * @param output 운영 체제 버전 정보를 저장할 OSVERSIONINFOW 구조체에 대한 포인터입니다.
 *               호출자는 이 구조체의 dwOSVersionInfoSize 멤버를 sizeof(OSVERSIONINFOW)로 설정해야 합니다.
 *
 * @return 성공하면 STATUS_SUCCESS를 반환합니다.
 *         output이 NULL이면 STATUS_INVALID_PARAMETER를 반환합니다.
 *         RtlGetVersion 함수 호출이 실패하면 해당 실패 상태를 반환합니다.
 */
NTSTATUS Get_OS_Versions(OSVERSIONINFOW* output) {
    if (output == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    // RtlGetVersion을 사용하여 OS 버전 정보를 가져옵니다.
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
        256, // 할당된 버퍼 크기 (바이트)
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