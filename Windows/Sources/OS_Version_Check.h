#ifndef OS_Ver_CHECK
#define OS_Ver_CHECK

#include <ntifs.h>

NTSTATUS OS_Version_Checker(PUCHAR* opt_output_os_info);
NTSTATUS Output_OS_info_with_alloc(PUCHAR* output_ansi, ULONG MajorVersion, ULONG MinorVersion, ULONG BuildVersion, ULONG OSVersionInfoSize, ULONG PlatformId);
VOID OS_info_free(PUCHAR os_info);
#endif // !OS_Ver_CHECK