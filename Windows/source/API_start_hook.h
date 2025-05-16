#ifndef API_HOOK_H
#include <ntifs.h>

// ImageLoad�κп��� ntdll.dll�� �ε�� �� ��ŷ�� ������ �õ��غ� �� �ִ�. ( �̶��� LoadLibraryA, W �ּҰ� ��ȿ�ϸ�, ������ ���μ����� ����Ǳ����� �غ�-�ܰ��� )

// ��ŷ Ŀ�� �Լ� -> RtlCreateUserThread
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