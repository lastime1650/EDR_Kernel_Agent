#include "Event_Manager.h"


// [1/] ���μ��� ���� �ڵ鷯
#include "Process_Creation_Event.h"

// [2/] ���μ��� ���� �ڵ鷯
#include "Process_Remove_Event.h"

// [3/] �̹��� �ε� �ڵ鷯
#include "Image_Load_Event.h"

// [4/] �̴����� �ڵ鷯
#include "MiniFilter.h"

// [5/] NDIS �ڵ鷯
#include "Network_Event.h"

// [6/] ������Ʈ�� �ڵ鷯
#include "Registry_Event.h"

// �̺�Ʈ �δ�
NTSTATUS Event_Load(
	PDRIVER_OBJECT Input_Driver_Obj,
	PDEVICE_OBJECT input_device_Obj
) {
	UNREFERENCED_PARAMETER(Input_Driver_Obj);
	UNREFERENCED_PARAMETER(input_device_Obj);
	NTSTATUS status;

	// [1/] ���μ��� ���� �ڵ鷯
	status = Process_Creation_Event_Loader(); // ���� ���� �߻� ������: ��������(����Ȯ�οϷ�)

	// [2/] ���μ��� ���� �ڵ鷯
	status = Process_Remove_Event_Loader(); // ���� ���� �߻� ������: x

	// [3/] �̹��� �ε� �ڵ鷯
	status = Image_Load_Event_Loader(); // ���� ���� �߻� ������: ��������(����Ȯ�οϷ�)

	// [4/] �̴����� �ڵ鷯
	status = Register_MiniFilter(Input_Driver_Obj); // ���� ���� �߻� ������: ��������(����Ȯ�οϷ�)

	// [5/] NDIS �ڵ鷯
	status = Network_Event_Loader(input_device_Obj); // ���� ���� �߻� ������: x

	// [6/] ������Ʈ�� �ڵ鷯
	status = Registry_Event_Loader(Input_Driver_Obj, L"390596"); // ���� ���� �߻� ������: ��������(����Ȯ�οϷ�) -> WORK_ITEM�� �������(*https://community.osr.com/t/is-this-a-bug-that-occurs-in-the-windows-operating-system-kernel/59355*)

	return status;
}

// �̺�Ʈ ��δ�
NTSTATUS Event_Unload() {
	NTSTATUS status;

	// [1/] ���μ��� ���� �ڵ鷯
	status = Process_Creation_Event_Unloader();

	// [2/] ���μ��� ���� �ڵ鷯
	status = Process_Remove_Event_Unloader();

	// [3/] �̹��� �ε� �ڵ鷯
	status = Image_Load_Event_Unloader();

	// [4/] �̴����� �ڵ鷯
	status = Unload_Minifilter();

	// [5/] NDIS �ڵ鷯
	/*... �ʹ� ����!! ���߿� ����...*/

	// [6/] ������Ʈ�� �ڵ鷯
	status = Registry_Event_Unloader();


	return status;
}