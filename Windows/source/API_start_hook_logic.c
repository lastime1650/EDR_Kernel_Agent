#include "API_start_hook.h"
//#include "API_Loader.h"
#include "query_process_information.h"

#include "Virtual_Alloc.h" // 유저 프로세스에 가상주소 할당

BOOLEAN CheckNtdllDll(PUNICODE_STRING FullImageName);
ULONG_PTR Get_LdrLoadDll_Address(ULONG_PTR ntdllBaseAddress);

typedef void(__stdcall* LdrLoadDll) (
	IN PWCHAR               PathToFile OPTIONAL,
	IN ULONG                Flags OPTIONAL,
	IN PUNICODE_STRING      ModuleFileName,
	OUT HANDLE* ModuleHandle);

NTSTATUS Lets_Hook(HANDLE processId, PUNICODE_STRING FullImageName, PVOID Dll_Start_Address) {

	if (!FullImageName)
		return STATUS_UNSUCCESSFUL;

	// 1.
	if (!CheckNtdllDll(FullImageName))
		return STATUS_UNSUCCESSFUL;

	// 2.
	PVOID LdrLoadDll_VirtualAddress = NULL;
	LdrLoadDll_VirtualAddress = (PVOID)Get_LdrLoadDll_Address((ULONG_PTR)Dll_Start_Address); // LdrLoadDll 주소 가져오기

	if (!LdrLoadDll_VirtualAddress)
		return FALSE;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " 주소: %p \n", LdrLoadDll_VirtualAddress);





	NTSTATUS status;

	HANDLE RealHandle = 0;
	status = PID_to_RealHandle(processId, &RealHandle);
	if (status != STATUS_SUCCESS || !RealHandle ) {
		return status;
	}

	PVOID LoadLibraryA_Address = LdrLoadDll_VirtualAddress;
	CHAR HookDLLPathA_Parameter_A[] = "C:\\EDR_HOOK_DLL.dll";


	PVOID LoadLibraryA_parameter_virtualaddress = NULL;
	SIZE_T output = 0;
	Virtual__Allocate_2_Process(
		RealHandle,

		sizeof(HookDLLPathA_Parameter_A),
		PAGE_EXECUTE_READWRITE,

		&LoadLibraryA_parameter_virtualaddress,
		&output
	);

	/* 추천하지 않은 작업 ( kestackattach ) */
	KAPC_STATE STATE;
	PEPROCESS eprocess = NULL;
	PsLookupProcessByProcessId(processId, &eprocess);
	KeStackAttachProcess(eprocess, &STATE);

	memcpy(LoadLibraryA_parameter_virtualaddress, HookDLLPathA_Parameter_A, sizeof(HookDLLPathA_Parameter_A));

	KeUnstackDetachProcess(&STATE);

	
	HANDLE output_ThreadHandle = 0;
	status = RtlCreateUserThread(
		RealHandle,//
		NULL,
		FALSE,
		0,
		0,
		0,
		LoadLibraryA_Address,//
		LoadLibraryA_parameter_virtualaddress,//
		&output_ThreadHandle,
		NULL
	);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RtlCreateUserThread 결과 ->> %p  \n", status);

	return status;

}


BOOLEAN CheckNtdllDll(PUNICODE_STRING FullPath)
{
	/// 1. 인자 유효성 검사: FullPath 포인터와 Buffer가 NULL이 아닌지 확인
	if (FullPath == NULL || FullPath->Buffer == NULL) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Invalid FullPath parameter!\n");
		return FALSE;
	}

	// 2. 마지막 경로 구분자 찾기
	PWCHAR lastSlash = wcsrchr(FullPath->Buffer, L'\\');
	if (lastSlash == NULL) {
		// '\'가 없는 경우 (예: "ntdll.dll")
		lastSlash = FullPath->Buffer;
	}
	else {
		lastSlash++;  // '\' 다음 문자부터 비교

		// 3. lastSlash가 범위를 벗어나는지 확인
		if (lastSlash > FullPath->Buffer + FullPath->Length / sizeof(WCHAR)) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "lastSlash out of range!\n");
			return FALSE;
		}
	}

	// 4. lastSlash가 여전히 유효한지 확인 (NULL 포인터 역참조 방지)
	if (lastSlash == NULL) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "lastSlash is NULL after processing!\n");
		return FALSE; // 또는 다른 오류 처리
	}

	UNICODE_STRING lastPart;

	RtlInitUnicodeString(&lastPart, lastSlash);  

	UNICODE_STRING targetString;
	RtlInitUnicodeString(&targetString, L"kernel32.dll");

	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DLL_NAME test -> %wZ \n", lastPart);

	// 대/소문자 구분 없이 비교
	if (RtlCompareUnicodeString(&lastPart, &targetString, TRUE) == 0) { // 유니코드비교 API 로 ntdll.dll 인지 확인
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "kernel32.dll 로드확인! ");

		return TRUE; // 일치
	}

	return FALSE; // 불일치
}

//

#include <ntimage.h>
#include <ntstrsafe.h>  //strcmp를 사용하려면 include

ULONG_PTR Get_LdrLoadDll_Address(ULONG_PTR ntdllBaseAddress) {
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ntdllBaseAddress;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		return 0; // Invalid DOS header
	}

	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((ULONG_PTR)ntdllBaseAddress + pDosHeader->e_lfanew);
	if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
		return 0; // Invalid NT header
	}

	PIMAGE_DATA_DIRECTORY pExportDataDir = &pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)((ULONG_PTR)ntdllBaseAddress + pExportDataDir->VirtualAddress);

	PULONG pAddressOfNames = (PULONG)((ULONG_PTR)ntdllBaseAddress + pExportDir->AddressOfNames);
	PULONG pAddressOfFunctions = (PULONG)((ULONG_PTR)ntdllBaseAddress + pExportDir->AddressOfFunctions);
	PUSHORT pAddressOfNameOrdinals = (PUSHORT)((ULONG_PTR)ntdllBaseAddress + pExportDir->AddressOfNameOrdinals);

	ULONG loadLibraryARVA = 0;
	for (ULONG i = 0; i < pExportDir->NumberOfNames; i++) {
		PCHAR functionName = (PCHAR)((ULONG_PTR)ntdllBaseAddress + pAddressOfNames[i]);

		//strcmp 대신 RtlCompareString 사용. (더욱 안전)
		ANSI_STRING funcName;
		RtlInitAnsiString(&funcName, functionName);
		ANSI_STRING targetFuncName;
		RtlInitAnsiString(&targetFuncName, "LoadLibraryA");

		if (RtlCompareString(&funcName, &targetFuncName, FALSE) == 0) // FALSE: 대/소문자 구분
		{
			USHORT ordinal = pAddressOfNameOrdinals[i];
			loadLibraryARVA = pAddressOfFunctions[ordinal];
			break;
		}

	}

	if (loadLibraryARVA != 0) {
		return (ULONG_PTR)ntdllBaseAddress + loadLibraryARVA;
	}

	return 0; // Not found
}