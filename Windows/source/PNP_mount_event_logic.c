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

	// 심볼릭
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

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " PNP 비동기 결과 문자열 -> %wZ \n", USB_DEVICE_STRING);
	KeSetEvent(&context->event, 0, FALSE);//대기중인 스레드 해제




	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "2 NotificationStructure->SymbolicLinkName -> %wZ \n", USB_DEVICE_SYMBOLIC_STRING);

	UNICODE_STRING get_SERIAL_NUM = { 0, };
	if (Get_USB_Serial(&USB_DEVICE_SYMBOLIC_STRING, &get_SERIAL_NUM, 2, FALSE)) {
		PALL_DEVICE_DRIVES output = NULL;
		while (output == NULL) {


			Delays(-1);


			// 마운트 시 
			if (is_unmount == FALSE) {
				// 전체 드라이브 정보 얻기 호출
				ListMountedDrives(&USB_DEVICE_STRING, FALSE);



				output = Finder_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node, NULL, &get_SERIAL_NUM, NULL); // 이미 연결리스트에 등록된 장치에서 찾기
			}

			// 언마운트 시
			if (is_unmount) {

			}

		}
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "3 output-> 시리얼 %wZ  \n", output->USBSTOR_Serial);


	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "4 NotificationStructure->SymbolicLinkName \n");



	ExFreePoolWithTag(USB_DEVICE_STRING.Buffer, 'DEST');
	ExFreePoolWithTag(USB_DEVICE_SYMBOLIC_STRING.Buffer, 'DEST');

}




// PNP등록
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

	// 장치 핸들 얻기
	HANDLE linkHandle = 0;

	// OBJECT_ATTRIBUTES 초기화
	OBJECT_ATTRIBUTES objAttr = { 0, };
	InitializeObjectAttributes(&objAttr, NotificationStructure->SymbolicLinkName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	NTSTATUS status = ZwOpenSymbolicLinkObject(&linkHandle, SYMBOLIC_LINK_QUERY, &objAttr);
	if (NT_SUCCESS(status)) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Device TRUE\n");

		MOVER parameter = { 0, };

		// 심볼릭
		parameter.USB_DEVICE_SYMBOLIC_STRING.Buffer = NotificationStructure->SymbolicLinkName->Buffer;
		parameter.USB_DEVICE_SYMBOLIC_STRING.Length = NotificationStructure->SymbolicLinkName->Length;
		parameter.USB_DEVICE_SYMBOLIC_STRING.MaximumLength = NotificationStructure->SymbolicLinkName->MaximumLength;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "1 NotificationStructure->SymbolicLinkName -> %wZ \n", parameter.USB_DEVICE_SYMBOLIC_STRING);


		WCHAR targetBuffer[MAXIMUM_FILENAME_LENGTH];
		parameter.USB_DEVICE_STRING.Buffer = targetBuffer;
		parameter.USB_DEVICE_STRING.Length = 0;
		parameter.USB_DEVICE_STRING.MaximumLength = sizeof(targetBuffer);


		status = ZwQuerySymbolicLinkObject(linkHandle, &parameter.USB_DEVICE_STRING, NULL); // parameter.USB_DEVICE_STRING값은 미리 할당된 공간이 있어야 작동된다.
		if (NT_SUCCESS(status)) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Device -- NT Path: %wZ\n", &parameter.USB_DEVICE_STRING);





			KeInitializeEvent(&parameter.event, SynchronizationEvent, FALSE); // 이벤트 초기화

			if (IsEqualGUID(&NotificationStructure->Event, &GUID_DEVICE_INTERFACE_ARRIVAL)) { // 마운트 시
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Device Arrival (마운트 중)\n");

				parameter.is_unmount = FALSE;

				// 마운트 중 처리
				// 일정 시간 대기

				HANDLE g_ThreadHandle = 0;
				// 스레드를 생성합니다.
				PsCreateSystemThread(
					&g_ThreadHandle,
					THREAD_ALL_ACCESS,
					NULL,
					NULL,
					NULL,
					Start_Drive_Scan_for_PNP,
					&parameter // 필요시 Context 포인터 사용
				);

				KeWaitForSingleObject(&parameter, Executive, KernelMode, FALSE, NULL); // 비동기 스레드에서 해제 대기중..

			}
			else if (IsEqualGUID(&NotificationStructure->Event, &GUID_DEVICE_INTERFACE_REMOVAL)) { // 언마운트 시 
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Device Removal (언마운트 중)\n");
				parameter.is_unmount = TRUE;
				//ListMountedDrives(FALSE, FALSE);
				// 언마운트 중 처리
			}







		}
		ZwClose(linkHandle);
	}


	return STATUS_SUCCESS;
}