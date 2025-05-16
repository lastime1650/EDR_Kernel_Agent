#ifndef Request_Real_File

#include <ntifs.h>

#include "Analysis_enum.h"

#include "DynamicData_Linked_list.h"
VOID PROCESS_Request_Real_File(
	PDynamicData input_recv_data,

	PUCHAR* out_send_data,
	ULONG32* out_send_data_size
);

#endif