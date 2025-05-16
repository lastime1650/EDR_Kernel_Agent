#include "PNP_mount_event.h"


typedef struct MOVER {
	KEVENT event;

	UNICODE_STRING USB_DEVICE_SYMBOLIC_STRING;
	UNICODE_STRING USB_DEVICE_STRING;


	BOOLEAN is_unmount;
}MOVER, * PMOVER;



VOID Start_Drive_Scan_for_PNP(PMOVER context) {

	UNICODE_STRING USB_DEVICE_STRING = { 0, };

	USB_DEVICE_STRING.Length = context->USB_DEVICE_STRING.Length;
	USB_DEVICE_STRING.MaximumLength = context->USB_DEVICE_STRING.MaximumLength;
	USB_DEVICE_STRING.Buffer = ExAllocatePoolWithTag(NonPagedPool, USB_DEVICE_STRING.MaximumLength, 'DEST');
	if (USB_DEVICE_STRING.Buffer == NULL) {
		ExFreePoolWithTag(USB_DEVICE_STRING.Buffer, 'DEST');
		return;
	}
	memcpy(USB_DEVICE_STRING.Buffer, context->USB_DEVICE_STRING.Buffer, USB_DEVICE_STRING.MaximumLength);

	// �ɺ���
	UNICODE_STRING USB_DEVICE_SYMBOLIC_STRING = { 0, };

	USB_DEVICE_SYMBOLIC_STRING.Length = context->USB_DEVICE_SYMBOLIC_STRING.Length;
	USB_DEVICE_SYMBOLIC_STRING.MaximumLength = context->USB_DEVICE_SYMBOLIC_STRING.MaximumLength;
	USB_DEVICE_SYMBOLIC_STRING.Buffer = ExAllocatePoolWithTag(NonPagedPool, USB_DEVICE_SYMBOLIC_STRING.MaximumLength, 'DEST');
	if (USB_DEVICE_SYMBOLIC_STRING.Buffer == NULL) {
		ExFreePoolWithTag(USB_DEVICE_STRING.Buffer, 'DEST');
		return;
	}
	memcpy(USB_DEVICE_SYMBOLIC_STRING.Buffer, context->USB_DEVICE_SYMBOLIC_STRING.Buffer, USB_DEVICE_SYMBOLIC_STRING.MaximumLength);

	BOOLEAN is_unmount = context->is_unmount;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " PNP �񵿱� ��� ���ڿ� -> %wZ \n", USB_DEVICE_STRING);
	KeSetEvent(&context->event, 0, FALSE);//������� ������ ����




	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "2 NotificationStructure->SymbolicLinkName -> %wZ \n", USB_DEVICE_SYMBOLIC_STRING);

	UNICODE_STRING get_SERIAL_NUM = { 0, };
	if (Get_USB_Serial(&USB_DEVICE_SYMBOLIC_STRING, &get_SERIAL_NUM, 2, FALSE)) {
		PALL_DEVICE_DRIVES output = NULL;
		while (output == NULL) {


			Delays(-1);


			// ����Ʈ �� 
			if (is_unmount == FALSE) {
				// ��ü ����̺� ���� ��� ȣ��
				ListMountedDrives(&USB_DEVICE_STRING, FALSE);



				output = Finder_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node, NULL, &get_SERIAL_NUM, NULL); // �̹� ���Ḯ��Ʈ�� ��ϵ� ��ġ���� ã��
			}

			// �𸶿�Ʈ ��
			if (is_unmount) {

			}

		}
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "3 output-> �ø��� %wZ  \n", output->USBSTOR_Serial);


	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "4 NotificationStructure->SymbolicLinkName \n");



	ExFreePoolWithTag(USB_DEVICE_STRING.Buffer, 'DEST');
	ExFreePoolWithTag(USB_DEVICE_SYMBOLIC_STRING.Buffer, 'DEST');

}




// PNP���
NTSTATUS PNP_Register(PDRIVER_OBJECT DriverObject) {

	PVOID notificationHandle;


	return IoRegisterPlugPlayNotification(
		EventCategoryDeviceInterfaceChange,
		PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
		(PVOID)&GUID_DEVINTERFACE_USB_DEVICE,
		DriverObject,
		(PDRIVER_NOTIFICATION_CALLBACK_ROUTINE)PNP__NotificationRoutine,
		NULL,
		&notificationHandle
	);

}


void printf_guid(GUID guid) {
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Guid = {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX} \n",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

NTSTATUS PNP__NotificationRoutine(
	_In_ PDEVICE_INTERFACE_CHANGE_NOTIFICATION NotificationStructure,
	_In_opt_ PVOID Context
)
{
	UNREFERENCED_PARAMETER(Context);


	printf_guid(NotificationStructure->Event);

	// ��ġ �ڵ� ���
	HANDLE linkHandle = 0;

	// OBJECT_ATTRIBUTES �ʱ�ȭ
	OBJECT_ATTRIBUTES objAttr = { 0, };
	InitializeObjectAttributes(&objAttr, NotificationStructure->SymbolicLinkName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	NTSTATUS status = ZwOpenSymbolicLinkObject(&linkHandle, SYMBOLIC_LINK_QUERY, &objAttr);
	if (NT_SUCCESS(status)) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Device TRUE\n");

		MOVER parameter = { 0, };

		// �ɺ���
		parameter.USB_DEVICE_SYMBOLIC_STRING.Buffer = NotificationStructure->SymbolicLinkName->Buffer;
		parameter.USB_DEVICE_SYMBOLIC_STRING.Length = NotificationStructure->SymbolicLinkName->Length;
		parameter.USB_DEVICE_SYMBOLIC_STRING.MaximumLength = NotificationStructure->SymbolicLinkName->MaximumLength;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "1 NotificationStructure->SymbolicLinkName -> %wZ \n", parameter.USB_DEVICE_SYMBOLIC_STRING);


		WCHAR targetBuffer[MAXIMUM_FILENAME_LENGTH];
		parameter.USB_DEVICE_STRING.Buffer = targetBuffer;
		parameter.USB_DEVICE_STRING.Length = 0;
		parameter.USB_DEVICE_STRING.MaximumLength = sizeof(targetBuffer);


		status = ZwQuerySymbolicLinkObject(linkHandle, &parameter.USB_DEVICE_STRING, NULL); // parameter.USB_DEVICE_STRING���� �̸� �Ҵ�� ������ �־�� �۵��ȴ�.
		if (NT_SUCCESS(status)) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Device -- NT Path: %wZ\n", &parameter.USB_DEVICE_STRING);





			KeInitializeEvent(&parameter.event, SynchronizationEvent, FALSE); // �̺�Ʈ �ʱ�ȭ

			if (IsEqualGUID(&NotificationStructure->Event, &GUID_DEVICE_INTERFACE_ARRIVAL)) { // ����Ʈ ��
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Device Arrival (����Ʈ ��)\n");

				parameter.is_unmount = FALSE;

				// ����Ʈ �� ó��
				// ���� �ð� ���

				HANDLE g_ThreadHandle = 0;
				// �����带 �����մϴ�.
				PsCreateSystemThread(
					&g_ThreadHandle,
					THREAD_ALL_ACCESS,
					NULL,
					NULL,
					NULL,
					Start_Drive_Scan_for_PNP,
					&parameter // �ʿ�� Context ������ ���
				);

				KeWaitForSingleObject(&parameter, Executive, KernelMode, FALSE, NULL); // �񵿱� �����忡�� ���� �����..

			}
			else if (IsEqualGUID(&NotificationStructure->Event, &GUID_DEVICE_INTERFACE_REMOVAL)) { // �𸶿�Ʈ �� 
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Device Removal (�𸶿�Ʈ ��)\n");
				parameter.is_unmount = TRUE;
				//ListMountedDrives(FALSE, FALSE);
				// �𸶿�Ʈ �� ó��
			}







		}
		ZwClose(linkHandle);
	}


	return STATUS_SUCCESS;
}