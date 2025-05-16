#ifndef my_ioctl_H
#define my_ioctl_H

#include <ntifs.h>
#include <ntddk.h>


#define DEVICE_NAME L"\\Device\\My_AGENT_Device"
#define SYMLINK_NAME L"\\??\\My_AGENT_Device"
#define IOCTL_TEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS) // General
#define IOCTL_by_API CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS) // APIÀü¿ë CODE

typedef struct IOCTL_STRUCT {
    CHAR API_NAME[50];
    HANDLE PID;
    PUCHAR BUFFER;
    ULONG32 BUFFER_SIZE;
}IOCTL_STRUCT, * PIOCTL_STRUCT;


PDEVICE_OBJECT DeviceInitialize(IN PDRIVER_OBJECT DriverObject);

NTSTATUS DriverUnload(_In_ PDRIVER_OBJECT DriverObject);

NTSTATUS CreateClose(PDEVICE_OBJECT pDeviceObject, PIRP Irp);

NTSTATUS IoControl(PDEVICE_OBJECT pDeviceObject, PIRP Irp);


#endif