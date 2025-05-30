#ifndef VIRTUAL_ALLOC

#include <ntifs.h>

// 프로세스에 가상 주소 할당
NTSTATUS Virtual__Allocate_2_Process(
	HANDLE RealHandle,
	SIZE_T Buffer_Size,

	ULONG32 protect,

	PVOID* output_virtual_address,
	SIZE_T* output_allocated_size
);


// 프로세스 할당된 주소 해제
NTSTATUS Virtual__FREE_2_Process(
	HANDLE RealHandle,

	PVOID input_allocated_virtual_address,
	SIZE_T input_allocated_size
);

#endif