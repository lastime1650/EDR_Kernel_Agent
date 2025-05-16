#ifndef VIRTUAL_ALLOC

#include <ntifs.h>

// ���μ����� ���� �ּ� �Ҵ�
NTSTATUS Virtual__Allocate_2_Process(
	HANDLE RealHandle,
	SIZE_T Buffer_Size,

	ULONG32 protect,

	PVOID* output_virtual_address,
	SIZE_T* output_allocated_size
);


// ���μ��� �Ҵ�� �ּ� ����
NTSTATUS Virtual__FREE_2_Process(
	HANDLE RealHandle,

	PVOID input_allocated_virtual_address,
	SIZE_T input_allocated_size
);

#endif