#ifndef PROCESS_REMOVE_EVENT
#include <ntifs.h>

/*
	프로세스 종료 핸들러
*/

// 로더
NTSTATUS Process_Remove_Event_Loader();

// 언로더
NTSTATUS Process_Remove_Event_Unloader();

// 핸들러
VOID PcreateProcessNotifyRoutine(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);

#endif