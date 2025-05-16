#include <ntifs.h>

#include "OS_Version_Check.h"
#include "API_Loader.h"
#include "my_ioctl.h"
#include "Event_Manager.h"
#include "Start_TCP_session.h"
#include "Obregistercallbacks_regtister.h"
#include "Get_Volumes.h"


VOID DriverUnload_(PDRIVER_OBJECT DriverObject);
NTSTATUS DriverEntry(PDRIVER_OBJECT driverobject, PUNICODE_STRING registry) {

	UNREFERENCED_PARAMETER(registry);

	NTSTATUS status = STATUS_SUCCESS;

	

	// [1/6] :: OS ���� üũ
	status = OS_Version_Checker(NULL);
	if (status != STATUS_SUCCESS)
		return status;

	// [2/6] :: API �ε� ( ȣȯ�� ���� )
	status = API_Loader();
	if (status != STATUS_SUCCESS)
		return status;

	// [3/6] :: IOCTL ����̽� ����
	PDEVICE_OBJECT deviceobj = NULL;
	deviceobj = DeviceInitialize(driverobject);
	if (!deviceobj)
		return STATUS_UNSUCCESSFUL;

	// [New] :: DLP �ַ�� ����!
	// 1. ���� ��ġ �˻� ( ���� �ϵ� �Ǵ� USB �˻� ) - ���ĺ� ( DOS )��� ó��
	ListMountedDrives(NULL, FALSE);

	// �׽�Ʈ ����Ʈ


	// [4/6] :: �̺�Ʈ ���
	status = Event_Load(driverobject, deviceobj);
	if (status != STATUS_SUCCESS)
		return status;

	// [New] :: ���ܱ� ��� 
	// - [���μ���-����] ObregisterCallbacks
	status = ObregisterCallbacks_Registering(L"17299");
	if (status != STATUS_SUCCESS)
		return status;

	

	// [5/6] :: ����̹� ���� ���
	driverobject->DriverUnload = DriverUnload_;
	
	// [6/6] :: �񵿱� EDR ���� ��ż���
	HANDLE thread;
	status = PsCreateSystemThread(
		&thread,
		THREAD_ALL_ACCESS,
		NULL,
		0,
		NULL,
		Loop_for_TCP_Session,
		NULL
	);
	
	return status;
}




// ����̹� ���� ����
VOID DriverUnload_(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	// ... ����̹� ���� �ڵ� ...

	// �̺�Ʈ ��� ����
	Event_Unload();

	// ����̽� ����
	UNICODE_STRING symlinkName;
	RtlInitUnicodeString(&symlinkName, SYMLINK_NAME);
	//
	IoDeleteSymbolicLink(&symlinkName);
	IoDeleteDevice(DriverObject->DeviceObject);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MyDriverUnload: ����̹��� �����Ǿ����ϴ�.\n");
}