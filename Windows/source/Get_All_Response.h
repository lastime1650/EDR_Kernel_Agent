#ifndef GET_ALL_RESPONSE_LIST
#define __GET_ALL_RESPONSE

#include <ntifs.h>

BOOLEAN Get_All_Response_datas(
	PUCHAR* out_send_data,
	ULONG32* out_send_data_size
);

#endif