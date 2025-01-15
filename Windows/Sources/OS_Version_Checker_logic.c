#include "OS_Version_Check.h"

// 윈도우 10의 주 버전 번호를 정의합니다.
#define WINDOWS_10_MAJOR_VERSION 10

/*
 * @brief 운영 체제의 버전을 확인하고 Windows 10 이상인지 여부를 반환합니다.
 *
 * @return Windows 10 이상이면 STATUS_SUCCESS를, 그렇지 않으면 STATUS_NOT_SUPPORTED를 반환합니다.
 *         RtlGetVersion 함수 호출이 실패하면 해당 실패 상태를 반환합니다.
 */
NTSTATUS OS_Version_Checker() {
    RTL_OSVERSIONINFOW osVersionInfo = { 0 };
    osVersionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

    // RtlGetVersion을 사용하여 OS 버전 정보를 가져옵니다.
    NTSTATUS status = RtlGetVersion(&osVersionInfo);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    // Windows 10 이상인지 확인합니다.
    if (osVersionInfo.dwMajorVersion >= WINDOWS_10_MAJOR_VERSION) {
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