#include "Virtual_Alloc.h"

// ���μ����� ���� �ּ� �Ҵ�
NTSTATUS Virtual__Allocate_2_Process(
	HANDLE RealHandle,
	SIZE_T Buffer_Size,

	ULONG32 protect,

	PVOID* output_virtual_address,
	SIZE_T* output_allocated_size
) {
	*output_virtual_address = NULL;
	*output_allocated_size = Buffer_Size;

	return ZwAllocateVirtualMemory(
		RealHandle,
		output_virtual_address,
		(ULONG_PTR)NULL,
		output_allocated_size,
		MEM_COMMIT,
		protect
	);

}


// ���μ��� �Ҵ�� �ּ� ����
NTSTATUS Virtual__FREE_2_Process(
	HANDLE RealHandle,

	PVOID input_allocated_virtual_address,
	SIZE_T input_allocated_size
) {
	return ZwFreeVirtualMemory(
		RealHandle,
		&input_allocated_virtual_address,
		&input_allocated_size,
		MEM_FREE
	);
}