#ifndef PROCESS_Request_Process_Response_H
#define PROCESS_Request_Process_Response_HH

#include "Response_Process.h"
VOID PROCESS_Request_Process_Response(
	PDynamicData Parsed_data,

	PUCHAR* out_send_data,
	ULONG32* out_send_data_size,

	BOOLEAN is_remove
);

#endif