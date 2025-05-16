#ifndef API_HOOK_H
#include <ntifs.h>

// ImageLoad부분에서 ntdll.dll이 로드될 때 후킹을 안전히 시도해볼 수 있다. ( 이때는 LoadLibraryA, W 주소가 유효하며, 완전히 프로세스가 실행되기전의 준비-단계임 )

// 후킹 커널 함수 -> RtlCreateUserThread
NTSTATUS NTAPI RtlCreateUserThread(
	HANDLE ProcessHandle,
	PSECURITY_DESCRIPTOR Security_Descriptor,
	BOOLEAN CreateSuspended,
	ULONG StackZeroBits,
	SIZE_T StackReserve,
	SIZE_T StackCommit,
	PVOID StartAddress,
	PVOID Parameter,
	PHANDLE ThreadHandle,
	PCLIENT_ID ClientId
);

NTSTATUS Lets_Hook(HANDLE processId, PUNICODE_STRING FullImageName, PVOID Dll_Start_Address);

#endif