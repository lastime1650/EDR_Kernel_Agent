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

#include "API_Loader.h"
NTSTATUS IoControl(PDEVICE_OBJECT pDeviceObject, PIRP Irp) {
	PIO_STACK_LOCATION pIoStackIrp = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	pIoStackIrp = IoGetCurrentIrpStackLocation(Irp);

	PVOID Input_Address = Irp->AssociatedIrp.SystemBuffer;
	PVOID output_Address = Irp->AssociatedIrp.SystemBuffer;

	switch (pIoStackIrp->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_TEST:
	{
		break;
	}
	case IOCTL_by_API:
	{
		/* API ��ŷ�� �༮���κ��� ������ */

		PIOCTL_STRUCT input_API_IOCTL_STRUCT = (PIOCTL_STRUCT)Input_Address;

		PCHAR API_NAME = input_API_IOCTL_STRUCT->API_NAME;
		HANDLE Sender_PID = input_API_IOCTL_STRUCT->PID;
		PUCHAR Sender_DynData_VirutalAddress = input_API_IOCTL_STRUCT->BUFFER;
		ULONG32 Sender_DynData_Size = input_API_IOCTL_STRUCT->BUFFER_SIZE;
		
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "{API_NAME: %s } [Sender_PID] --> %llu \n", API_NAME, Sender_PID);

		// Ŀ�� �����ּҷ� �������
		PUCHAR DynData = ExAllocatePoolWithTag(NonPagedPool, Sender_DynData_Size, 'API');

		// BUFFER�� "Ư�� ���μ����� �����ּ�"�̹Ƿ� �����ͼ� ó���ؾ���
		PEPROCESS User_eprocess = NULL;
		PsLookupProcessByProcessId(Sender_PID, &User_eprocess);

		/*
		KAPC_STATE apc = { 0 };
		KeStackAttachProcess(eprocess, &apc);

		RtlCopyMemory(DynData, Sender_DynData_VirutalAddress, Sender_DynData_Size);

		KeUnstackDetachProcess(&apc);
		*/
		SIZE_T output = 0;
		MmCopyVirtualMemory(
			User_eprocess,
			Sender_DynData_VirutalAddress,
			PsGetCurrentProcess(),
			DynData,
			Sender_DynData_Size,
			KernelMode,
			&output
		);

		ExFreePoolWithTag(DynData, 'API');

		output_Address = NULL;
		Irp->IoStatus.Information = 0;
		break;
	}
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);// ����


	return status;
}