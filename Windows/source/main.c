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

	

	// [1/6] :: OS 버전 체크
	status = OS_Version_Checker(NULL);
	if (status != STATUS_SUCCESS)
		return status;

	// [2/6] :: API 로드 ( 호환성 검토 )
	status = API_Loader();
	if (status != STATUS_SUCCESS)
		return status;

	// [3/6] :: IOCTL 디바이스 생성
	PDEVICE_OBJECT deviceobj = NULL;
	deviceobj = DeviceInitialize(driverobject);
	if (!deviceobj)
		return STATUS_UNSUCCESSFUL;

	// [New] :: DLP 솔루션 도입!
	// 1. 현재 장치 검색 ( 외장 하드 또는 USB 검색 ) - 알파벳 ( DOS )기반 처리
	ListMountedDrives(NULL, FALSE);

	// 테스트 프린트


	// [4/6] :: 이벤트 등록
	status = Event_Load(driverobject, deviceobj);
	if (status != STATUS_SUCCESS)
		return status;

	// [New] :: 차단기 등록 
	// - [프로세스-차단] ObregisterCallbacks
	status = ObregisterCallbacks_Registering(L"17299");
	if (status != STATUS_SUCCESS)
		return status;

	

	// [5/6] :: 드라이버 해제 등록
	driverobject->DriverUnload = DriverUnload_;
	
	// [6/6] :: 비동기 EDR 서버 통신수행
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




// 드라이버 해제 구현
VOID DriverUnload_(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	// ... 드라이버 해제 코드 ...

	// 이벤트 모두 해제
	Event_Unload();

	// 디바이스 해제
	UNICODE_STRING symlinkName;
	RtlInitUnicodeString(&symlinkName, SYMLINK_NAME);
	//
	IoDeleteSymbolicLink(&symlinkName);
	IoDeleteDevice(DriverObject->DeviceObject);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MyDriverUnload: 드라이버가 해제되었습니다.\n");
}