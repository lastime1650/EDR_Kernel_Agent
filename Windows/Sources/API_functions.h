#ifndef API_Function
#define API_Function

#include "API_structs.h"
#include <ntifs.h>

/* 

	�Լ� ������ ����ü

*/

// ZwQuerySystemInformation
typedef NTSTATUS(*Bring_ZwQuerySystemInformation)(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength
);


// ZwQueryInformationProcess
typedef NTSTATUS(*Bring_ZwQueryInformationProcess)(
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
	PULONG ReturnLength
);

// PsIsSystemProcess
typedef BOOLEAN(*Bring_PsIsSystemProcess)(
	PEPROCESS Process
	);

/*

	���� �Լ� ������ 

*/
extern Bring_ZwQuerySystemInformation ZwQuerySystemInformation;
extern Bring_ZwQueryInformationProcess ZwQueryInformationProcess;
extern Bring_PsIsSystemProcess PsIsSystemProcess;

#endif // !API_Function