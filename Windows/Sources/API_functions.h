#ifndef API_Function
#define API_Function

#include "API_structs.h"
#include <ntifs.h>

/* 

	함수 포인터 구조체

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

	전역 함수 포인터 

*/
extern Bring_ZwQuerySystemInformation ZwQuerySystemInformation;
extern Bring_ZwQueryInformationProcess ZwQueryInformationProcess;
extern Bring_PsIsSystemProcess PsIsSystemProcess;

#endif // !API_Function