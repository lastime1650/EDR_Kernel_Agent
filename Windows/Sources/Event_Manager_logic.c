#include "Event_Manager.h"


// [1/] 프로세스 생성 핸들러
#include "Process_Creation_Event.h"

// [2/] 프로세스 종료 핸들러
#include "Process_Remove_Event.h"

// [3/] 이미지 로드 핸들러
#include "Image_Load_Event.h"

// [4/] 미니필터 핸들러
#include "MiniFilter.h"

// [5/] NDIS 핸들러
#include "Network_Event.h"

// [6/] 레지스트리 핸들러
#include "Registry_Event.h"

// 이벤트 로더
NTSTATUS Event_Load(
	PDRIVER_OBJECT Input_Driver_Obj,
	PDEVICE_OBJECT input_device_Obj
) {
	UNREFERENCED_PARAMETER(Input_Driver_Obj);
	UNREFERENCED_PARAMETER(input_device_Obj);
	NTSTATUS status;

	// [1/] 프로세스 생성 핸들러
	status = Process_Creation_Event_Loader(); // 누수 부하 발생 검토결과: 문제없음(동적확인완료)

	// [2/] 프로세스 종료 핸들러
	status = Process_Remove_Event_Loader(); // 누수 부하 발생 검토결과: x

	// [3/] 이미지 로드 핸들러
	status = Image_Load_Event_Loader(); // 누수 부하 발생 검토결과: 문제없음(동적확인완료)

	// [4/] 미니필터 핸들러
	status = Register_MiniFilter(Input_Driver_Obj); // 누수 부하 발생 검토결과: 문제없음(동적확인완료)

	// [5/] NDIS 핸들러
	status = Network_Event_Loader(input_device_Obj); // 누수 부하 발생 검토결과: x

	// [6/] 레지스트리 핸들러
	status = Registry_Event_Loader(Input_Driver_Obj, L"390596"); // 누수 부하 발생 검토결과: 문제없음(동적확인완료) -> WORK_ITEM을 써야했음(*https://community.osr.com/t/is-this-a-bug-that-occurs-in-the-windows-operating-system-kernel/59355*)

	return status;
}

// 이벤트 언로더
NTSTATUS Event_Unload() {
	NTSTATUS status;

	// [1/] 프로세스 생성 핸들러
	status = Process_Creation_Event_Unloader();

	// [2/] 프로세스 종료 핸들러
	status = Process_Remove_Event_Unloader();

	// [3/] 이미지 로드 핸들러
	status = Image_Load_Event_Unloader();

	// [4/] 미니필터 핸들러
	status = Unload_Minifilter();

	// [5/] NDIS 핸들러
	/*... 너무 복잡!! 나중에 하자...*/

	// [6/] 레지스트리 핸들러
	status = Registry_Event_Unloader();


	return status;
}