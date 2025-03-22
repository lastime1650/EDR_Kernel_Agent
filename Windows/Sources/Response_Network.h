#ifndef RESPONSE_NETWORK_H
#define RESPONSE_NETWORK_H

#include "DynamicData_Linked_list.h"

#define HASH_LEN 64 // PUCHAR �ؽñ��� null ���ʿ�


typedef PDynamicData NetworkResponseData;


/*
	*************[ �˾ƾ� �� �� ]*************************

	PUCHAR�� ProcessResponse_Struct ������ ����ü

*/

extern NetworkResponseData Network_Response_Start_Node_Address;
extern NetworkResponseData Network_Response_Current_Node_Address;


BOOLEAN Append_Network_Response_Data(PCHAR RemoteIP, NetworkResponseData* opt_output_start_node); // ���� ��� �߰� ( ��, �ߺ� ���� )
BOOLEAN Remove_Networks_Reponse_Data(PCHAR RemoteIP, NetworkResponseData* opt_output_start_node); // ���� ��� ���� ( 1���� )
BOOLEAN is_exist_Network_Response_Data(PCHAR RemoteIP, ULONG32 RemoteIP_str_len_with_null); // ���� ��� Ȯ�� + ���� ���� ��

BOOLEAN Get_All_Response_list_Network_Response_Data__with_alloc(PDynamicData* output_start_buffer, PDynamicData* output_current_buffer);

VOID Print_Network_Response_Data_Nodes();

#endif