#pragma warning(disable:4100)
#include "my_ioctl.h"


PDEVICE_OBJECT DeviceInitialize(IN PDRIVER_OBJECT DriverObject) {


	if (!DriverObject) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DriverObject�� ������ϴ�.\n");
		return NULL;
	}

	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = NULL;
	UNICODE_STRING deviceName, symlinkName;

	RtlInitUnicodeString(&deviceName, DEVICE_NAME);
	RtlInitUnicodeString(&symlinkName, SYMLINK_NAME);

	// ����̽� ��ü ���� 
	status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	if (!NT_SUCCESS(status)) {
		return NULL;
	}
	// �ɺ��� ��ũ ���� - ������忡�� Ŀ�ο� �����ϱ� ����.. 
	status = IoCreateSymbolicLink(&symlinkName, &deviceName);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(pDeviceObject);
		return NULL;
	}


	//DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;

	pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING; // Ȱ��ȭ 

	return pDeviceObject;
}




// ����̽� ���ٽ� ó���ϴ� �Լ� ( ������� ������, IOCTL �� �⺻���� �ʿ� ) 
NTSTATUS CreateClose(PDEVICE_OBJECT pDeviceObject, PIRP Irp) {
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


NTSTATUS IoControl(PDEVICE_OBJECT pDeviceObject, PIRP Irp) {
	PIO_STACK_LOCATION pIoStackIrp = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	pIoStackIrp = IoGetCurrentIrpStackLocation(Irp);

	switch (pIoStackIrp->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_TEST:

		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);// ����


	return status;
}