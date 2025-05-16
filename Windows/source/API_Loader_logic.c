#include "API_Loader.h"

extern Bring_ZwQuerySystemInformation ZwQuerySystemInformation = NULL;
extern Bring_ZwQueryInformationProcess ZwQueryInformationProcess = NULL;
extern Bring_PsIsSystemProcess PsIsSystemProcess = NULL;
extern Bring_PsGetProcessInheritedFromUniqueProcessId PsGetProcessInheritedFromUniqueProcessId = NULL;
NTSTATUS API_Loader() {

	UNICODE_STRING API_NAME = { 0, };
	
	//ZwQuerySystemInformation
	RtlInitUnicodeString(&API_NAME, L"ZwQuerySystemInformation");
	ZwQuerySystemInformation = (Bring_ZwQuerySystemInformation)MmGetSystemRoutineAddress(&API_NAME);
	if (ZwQuerySystemInformation == NULL) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ZwQuerySystemInformation 실패 \n");
		return STATUS_NOT_SUPPORTED;
	}
	
	//ZwQueryInformationProcess
	RtlInitUnicodeString(&API_NAME, L"ZwQueryInformationProcess");
	ZwQueryInformationProcess = (Bring_ZwQueryInformationProcess)MmGetSystemRoutineAddress(&API_NAME);
	if (ZwQueryInformationProcess == NULL) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ZwQueryInformationProcess 실패 \n");
		return STATUS_NOT_SUPPORTED;
	}

	//PsIsSystemProcess
	RtlInitUnicodeString(&API_NAME, L"PsIsSystemProcess");
	PsIsSystemProcess = (Bring_PsIsSystemProcess)MmGetSystemRoutineAddress(&API_NAME);
	if (PsIsSystemProcess == NULL) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PsIsSystemProcess 실패 \n");
		return STATUS_NOT_SUPPORTED;
	}

	//PsGetProcessInheritedFromUniqueProcessId
	RtlInitUnicodeString(&API_NAME, L"PsGetProcessInheritedFromUniqueProcessId");
	PsGetProcessInheritedFromUniqueProcessId = (Bring_PsGetProcessInheritedFromUniqueProcessId)MmGetSystemRoutineAddress(&API_NAME);
	if (PsGetProcessInheritedFromUniqueProcessId == NULL) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PsGetProcessInheritedFromUniqueProcessId 실패 \n");
		return STATUS_NOT_SUPPORTED;
	}

	return STATUS_SUCCESS;
}