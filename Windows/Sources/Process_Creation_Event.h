#ifndef PROCESS_CREAETION_EVENT
#include <ntifs.h>

/*
	���μ��� ���� �ڵ鷯
*/

// �δ�
NTSTATUS Process_Creation_Event_Loader();

// ��δ�
NTSTATUS Process_Creation_Event_Unloader();

// �ڵ鷯
VOID PcreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo);

#endif