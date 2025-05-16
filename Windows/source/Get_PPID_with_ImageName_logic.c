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
	PsLookupProcessByProcessId(child_processId, &Child_eprocess); // �ڽ� eprocess ���ϱ�  (���� ����) 

	*output_parent_pid = 0;

	// ���� PPID�� ���Ѵ�.
	*output_parent_pid = PsGetProcessInheritedFromUniqueProcessId(
		Child_eprocess
	);

	ObDereferenceObject(Child_eprocess); //  �ڽ� eprocess ���� ����  (���� ���� -> ����) 

	if (*output_parent_pid == 0)
		return STATUS_UNSUCCESSFUL;

	// �θ� PID�� EPROCESS ����ü ���� ( ���� ���� )
	PEPROCESS parent_eprocess = NULL;
	if (PsLookupProcessByProcessId(*output_parent_pid, &parent_eprocess) != STATUS_SUCCESS)
		return STATUS_UNSUCCESSFUL;

	// �θ� PID�κ��� ImageName�� ���Ѵ� 
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