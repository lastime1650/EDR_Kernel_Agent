#ifndef PROCESS_REMOVE_EVENT
#include <ntifs.h>

/*
	���μ��� ���� �ڵ鷯
*/

// �δ�
NTSTATUS Process_Remove_Event_Loader();

// ��δ�
NTSTATUS Process_Remove_Event_Unloader();

// �ڵ鷯
VOID PcreateProcessNotifyRoutine(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);

#endif