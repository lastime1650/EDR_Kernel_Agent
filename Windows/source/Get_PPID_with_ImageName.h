#ifndef GET_PPID_WITH_IMAGENAME

#include "ntifs.h"

NTSTATUS Get_PPID_with_ImageName(
	HANDLE child_processId,
	PUNICODE_STRING* output_ImageName_with_allocated,
	PHANDLE output_parent_pid
);

VOID FREE_get_PPID_with_ImageName(
	PUNICODE_STRING input_ImageName_with_allocated
);

#endif