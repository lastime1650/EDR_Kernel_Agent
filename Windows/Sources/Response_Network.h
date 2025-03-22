#ifndef RESPONSE_NETWORK_H
#define RESPONSE_NETWORK_H

#include "DynamicData_Linked_list.h"

#define HASH_LEN 64 // PUCHAR 해시길이 null 불필요


typedef PDynamicData NetworkResponseData;


/*
	*************[ 알아야 할 것 ]*************************

	PUCHAR은 ProcessResponse_Struct 포인터 구조체

*/

extern NetworkResponseData Network_Response_Start_Node_Address;
extern NetworkResponseData Network_Response_Current_Node_Address;


BOOLEAN Append_Network_Response_Data(PCHAR RemoteIP, NetworkResponseData* opt_output_start_node); // 차단 목록 추가 ( 단, 중복 금지 )
BOOLEAN Remove_Networks_Reponse_Data(PCHAR RemoteIP, NetworkResponseData* opt_output_start_node); // 차단 목록 제거 ( 1개씩 )
BOOLEAN is_exist_Network_Response_Data(PCHAR RemoteIP, ULONG32 RemoteIP_str_len_with_null); // 차단 목록 확인 + 차단 여부 용

BOOLEAN Get_All_Response_list_Network_Response_Data__with_alloc(PDynamicData* output_start_buffer, PDynamicData* output_current_buffer);

VOID Print_Network_Response_Data_Nodes();

#endif