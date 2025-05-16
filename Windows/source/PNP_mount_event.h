#ifndef PNP_MOUNT_MANAGEMENT_H
#define PNP_MOUNT_MANAGEMENT_H

#include <ntifs.h>

#include "Get_Volumes.h"

#include "util_Delay.h"

// GUID
#include <initguid.h>
#include <devguid.h>    // ��ġ GUID ���� ��� ����
#include <wdmguid.h>

DEFINE_GUID(GUID_DEVINTERFACE_DISK,
	0x53f56307, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);

DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE,
	0xA5DCBF10, 0x6530, 0x11D2, 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed);


// PNP ��� �Լ�
NTSTATUS PNP_Register(PDRIVER_OBJECT DriverObject);

// PNP ����Ʈ �ڵ鷯
NTSTATUS PNP__NotificationRoutine(
	_In_ PDEVICE_INTERFACE_CHANGE_NOTIFICATION NotificationStructure,
	_In_opt_ PVOID Context
);

#endif