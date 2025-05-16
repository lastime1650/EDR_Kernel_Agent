#pragma warning(disable:4100)
#include "my_ioctl.h"


PDEVICE_OBJECT DeviceInitialize(IN PDRIVER_OBJECT DriverObject) {


	if (!DriverObject) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DriverObject가 비었습니다.\n");
		return NULL;
	}

	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = NULL;
	UNICODE_STRING deviceName, symlinkName;

	RtlInitUnicodeString(&deviceName, DEVICE_NAME);
	RtlInitUnicodeString(&symlinkName, SYMLINK_NAME);

	// 디바이스 객체 생성 
	status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	if (!NT_SUCCESS(status)) {
		return NULL;
	}
	// 심볼릭 링크 생성 - 유저모드에서 커널에 접근하기 위한.. 
	status = IoCreateSymbolicLink(&symlinkName, &deviceName);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(pDeviceObject);
		return NULL;
	}


	//DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;

	pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING; // 활성화 

	return pDeviceObject;
}




// 디바이스 접근시 처리하는 함수 ( 사용하지 않지만, IOCTL 시 기본으로 필요 ) 
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
		/* API 후킹한 녀석으로부터 가져옴 */

		PIOCTL_STRUCT input_API_IOCTL_STRUCT = (PIOCTL_STRUCT)Input_Address;

		PCHAR API_NAME = input_API_IOCTL_STRUCT->API_NAME;
		HANDLE Sender_PID = input_API_IOCTL_STRUCT->PID;
		PUCHAR Sender_DynData_VirutalAddress = input_API_IOCTL_STRUCT->BUFFER;
		ULONG32 Sender_DynData_Size = input_API_IOCTL_STRUCT->BUFFER_SIZE;
		
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "{API_NAME: %s } [Sender_PID] --> %llu \n", API_NAME, Sender_PID);

		// 커널 가상주소로 덮어야함
		PUCHAR DynData = ExAllocatePoolWithTag(NonPagedPool, Sender_DynData_Size, 'API');

		// BUFFER는 "특정 프로세스의 가상주소"이므로 가져와서 처리해야함
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
	IoCompleteRequest(Irp, IO_NO_INCREMENT);// 전송


	return status;
}