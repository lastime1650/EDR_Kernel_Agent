#ifndef PROCESS_Req_Network_H
#define PROCESS_Req_Network_H

#include <ntifs.h>
#include "Response_Network.h"
VOID PROCESS_Request_Network_Response(PDynamicData parsed_data, PUCHAR* output_send_data, ULONG32* output_send_data_len, BOOLEAN is_remove);

#endif