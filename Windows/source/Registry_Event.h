#ifndef Registry_Event

#include <ntifs.h>

/*
	������Ʈ�� �ڵ鷯
*/

// �δ�
NTSTATUS Registry_Event_Loader(PDRIVER_OBJECT input_driverobj, PWCH altitude_val);

// ��δ�
NTSTATUS Registry_Event_Unloader();

// �ڵ鷯
NTSTATUS ExRegistryCallback_for_monitor(PVOID CallbackContext, PVOID Argument1, PVOID Argument2);

#endif