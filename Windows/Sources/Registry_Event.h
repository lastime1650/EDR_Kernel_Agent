#ifndef Registry_Event

#include <ntifs.h>

/*
	레지스트리 핸들러
*/

// 로더
NTSTATUS Registry_Event_Loader(PDRIVER_OBJECT input_driverobj, PWCH altitude_val);

// 언로더
NTSTATUS Registry_Event_Unloader();

// 핸들러
NTSTATUS ExRegistryCallback_for_monitor(PVOID CallbackContext, PVOID Argument1, PVOID Argument2);

#endif