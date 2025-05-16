#include "Get_PPID_with_ImageName.h"
#include "API_functions.h"

NTSTATUS Get_PPID_with_ImageName(
	HANDLE child_processId,
	PUNICODE_STRING* output_ImageName_with_allocated,
	PHANDLE output_parent_pid
) {
	if (output_ImageName_with_allocated == NULL || output_parent_pid == NULL)
		return STATUS_INVALID_PARAMETER;

	PEPROCESS Child_eprocess = NULL;
	PsLookupProcessByProcessId(child_processId, &Child_eprocess); // 자식 eprocess 구하기  (참조 증가) 

	*output_parent_pid = 0;

	// 먼저 PPID를 구한다.
	*output_parent_pid = PsGetProcessInheritedFromUniqueProcessId(
		Child_eprocess
	);

	ObDereferenceObject(Child_eprocess); //  자식 eprocess 참조 감소  (참조 감소 -> 복구) 

	if (*output_parent_pid == 0)
		return STATUS_UNSUCCESSFUL;

	// 부모 PID의 EPROCESS 구조체 습득 ( 참조 증가 )
	PEPROCESS parent_eprocess = NULL;
	if (PsLookupProcessByProcessId(*output_parent_pid, &parent_eprocess) != STATUS_SUCCESS)
		return STATUS_UNSUCCESSFUL;

	// 부모 PID로부터 ImageName을 구한다 
	if (SeLocateProcessImageName(
		parent_eprocess,
		output_ImageName_with_allocated
	) != STATUS_SUCCESS)
		return STATUS_UNSUCCESSFUL;

	ObDereferenceObject(parent_eprocess);

	
	return STATUS_SUCCESS;
}

VOID FREE_get_PPID_with_ImageName(
	PUNICODE_STRING input_ImageName_with_allocated
) {
	if (input_ImageName_with_allocated)
		ExFreePool(input_ImageName_with_allocated);
}