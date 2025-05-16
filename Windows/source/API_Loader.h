#ifndef API_LOAD
#define API_LOAD

#include <ntifs.h>
#include <ntstrsafe.h>
#include "API_functions.h"

NTSTATUS
MmCopyVirtualMemory(
    _In_ PEPROCESS SourceProcess,
    _In_ PVOID SourceAddress,
    _In_ PEPROCESS TargetProcess,
    _In_ PVOID TargetAddress,
    _In_ SIZE_T BufferSize,
    _In_ KPROCESSOR_MODE PreviousMode,
    _Out_opt_ PSIZE_T ReturnSize
);

NTSTATUS API_Loader();

#endif // !API_LOAD
