#ifndef EVENT_STARTER
#include <ntifs.h>

NTSTATUS Event_Load(
	PDRIVER_OBJECT Input_Driver_Obj,
	PDEVICE_OBJECT input_device_Obj
);

NTSTATUS Event_Unload();

#endif