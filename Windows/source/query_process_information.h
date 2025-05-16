#ifndef query_process_information_H
#define query_process_information_H

#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>


#include "API_Loader.h"// ƒı∏Æ API¿÷¿Ω

NTSTATUS PID_to_RealHandle(HANDLE PID, HANDLE* ProcessHandle);

NTSTATUS Query_Process_info(
	HANDLE PID, 
	PROCESSINFOCLASS input_Process_class,
	PUNICODE_STRING* output_unicode
);
VOID Query_Process_Image_Name___Release_Free_POOL(PUNICODE_STRING INPUT_Unicode);


typedef struct Terminate_Sturct {
	
	BOOLEAN is_needs_terminate;
	
	ULONG32 FIle_Size;
	PCHAR SHA256;

}Terminate_Sturct, *PTerminate_Sturct;

NTSTATUS Query_Running_Processes__(
	PTerminate_Sturct opt_Input_info
);


#endif