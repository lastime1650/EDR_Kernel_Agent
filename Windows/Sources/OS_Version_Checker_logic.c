#include "OS_Version_Check.h"

// ������ 10�� �� ���� ��ȣ�� �����մϴ�.
#define WINDOWS_10_MAJOR_VERSION 10

/*
 * @brief � ü���� ������ Ȯ���ϰ� Windows 10 �̻����� ���θ� ��ȯ�մϴ�.
 *
 * @return Windows 10 �̻��̸� STATUS_SUCCESS��, �׷��� ������ STATUS_NOT_SUPPORTED�� ��ȯ�մϴ�.
 *         RtlGetVersion �Լ� ȣ���� �����ϸ� �ش� ���� ���¸� ��ȯ�մϴ�.
 */
NTSTATUS OS_Version_Checker() {
    RTL_OSVERSIONINFOW osVersionInfo = { 0 };
    osVersionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

    // RtlGetVersion�� ����Ͽ� OS ���� ������ �����ɴϴ�.
    NTSTATUS status = RtlGetVersion(&osVersionInfo);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    // Windows 10 �̻����� Ȯ���մϴ�.
    if (osVersionInfo.dwMajorVersion >= WINDOWS_10_MAJOR_VERSION) {
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