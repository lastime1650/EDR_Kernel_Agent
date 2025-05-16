#ifndef PROCESS_CREAETION_EVENT
#include <ntifs.h>

/*
	프로세스 생성 핸들러
*/

// 로더
NTSTATUS Process_Creation_Event_Loader();

// 언로더
NTSTATUS Process_Creation_Event_Unloader();

// 핸들러
VOID PcreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo);

#endif