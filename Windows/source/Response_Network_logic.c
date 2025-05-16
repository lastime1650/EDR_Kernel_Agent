#include "Response_Network.h"
#include "KEVENT_or_KMUTEX_init.h"
K_EVENT_or_MUTEX_struct mutex_for_a_networkresponse = { NULL, K_MUTEX, FALSE };

NetworkResponseData Network_Response_Start_Node_Address = NULL;
NetworkResponseData Network_Response_Current_Node_Address = NULL;

// RemoteIP 입력된 문자열은 꼭 null이 포함되어 있어야한다. 
BOOLEAN Append_Network_Response_Data(PCHAR RemoteIP,NetworkResponseData* opt_output_start_node) { // 차단 목록 추가 ( 단, 중복 금지 )

	K_object_init_check_also_lock_ifyouwant(&mutex_for_a_networkresponse, TRUE);


	if (Network_Response_Start_Node_Address == NULL) {
		Network_Response_Start_Node_Address = CreateDynamicData((PUCHAR)RemoteIP, (ULONG32)strlen(RemoteIP)+1 );
		Network_Response_Current_Node_Address = Network_Response_Start_Node_Address;
	}
	else {
		// 중복검사
		if (is_exist_Network_Response_Data(RemoteIP, (ULONG32)strlen(RemoteIP)+1)) {
			K_object_lock_Release(&mutex_for_a_networkresponse);
			return FALSE;
		}
		else {
			Network_Response_Current_Node_Address = AppendDynamicData(Network_Response_Current_Node_Address, (PUCHAR)RemoteIP, (ULONG32)strlen(RemoteIP) + 1);
		}
	}

	if (opt_output_start_node)
		*opt_output_start_node = Network_Response_Start_Node_Address;
	Print_Network_Response_Data_Nodes();
	K_object_lock_Release(&mutex_for_a_networkresponse);
	return TRUE;
}
BOOLEAN Remove_Networks_Reponse_Data(PCHAR RemoteIP, NetworkResponseData* opt_output_start_node) { // 차단 목록 제거 ( 1개씩 )
	if (Network_Response_Start_Node_Address == NULL)
		return FALSE;

	K_object_init_check_also_lock_ifyouwant(&mutex_for_a_networkresponse, TRUE); // 상호배제

	NetworkResponseData remember_previous_node = NULL;
	NetworkResponseData current = Network_Response_Start_Node_Address;
	while (current) {

		if ( 
			( strlen((PCHAR)current->Data) == strlen(RemoteIP) ) && 
			(memcmp((PCHAR)current->Data, RemoteIP, strlen(RemoteIP) ) == 0)

			) {

			NetworkResponseData tmp = NULL;

			// 노드 수정 해야한다. 
			if (current == Network_Response_Start_Node_Address) {
				tmp = current;
				Network_Response_Start_Node_Address = (NetworkResponseData)current->Next_Addr;
				if (current->Next_Addr) {
					Network_Response_Current_Node_Address = Network_Response_Start_Node_Address;
				}
				else {
					Network_Response_Current_Node_Address = NULL;
				}

			}
			else if (current == Network_Response_Current_Node_Address) {
				tmp = current;
				if (remember_previous_node) {
					remember_previous_node->Next_Addr = NULL;
					Network_Response_Current_Node_Address = remember_previous_node;
				}
				else {
					Network_Response_Start_Node_Address = NULL;
					Network_Response_Current_Node_Address = NULL;
				}

			}
			else {
				tmp = current;
				if (remember_previous_node == Network_Response_Start_Node_Address) {
					remember_previous_node = (NetworkResponseData)current->Next_Addr;
					if (!current->Next_Addr) {
						Network_Response_Current_Node_Address = NULL;
					}
				}
				else if (!current->Next_Addr) {
					if (remember_previous_node) {
						remember_previous_node->Next_Addr = NULL;
					}
					Network_Response_Current_Node_Address = remember_previous_node;
				}
				else {
					if (remember_previous_node)
						remember_previous_node->Next_Addr = current->Next_Addr;
				}

			}
			// 노드 해제
			ExFreePoolWithTag(tmp->Data, 'BUFF');
			ExFreePoolWithTag(tmp, 'NODE');

			if (opt_output_start_node)
				*opt_output_start_node = Network_Response_Start_Node_Address;
			K_object_lock_Release(&mutex_for_a_networkresponse);
			return TRUE;
		}
		remember_previous_node = current;
		current = (NetworkResponseData)current->Next_Addr;
	}

	K_object_lock_Release(&mutex_for_a_networkresponse);
	return FALSE;
}

BOOLEAN is_exist_Network_Response_Data(PCHAR RemoteIP, ULONG32 RemoteIP_str_len_with_null) { // 차단 목록 확인 용 + 차단 식별용
	if (Network_Response_Start_Node_Address == NULL)
		return FALSE;

	NetworkResponseData current = Network_Response_Start_Node_Address;
	while (current) {


		// 길이가 같은가?
		if (RemoteIP_str_len_with_null == current->Size) {
			// 서로 문자열이 같은가?
			if (memcmp(current->Data, RemoteIP, RemoteIP_str_len_with_null-1) == 0) {
				//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Network 차단! %s\n", (PCHAR)RemoteIP);
				return TRUE;
			}
		}

		current = (NetworkResponseData)current->Next_Addr;
	}

	return FALSE;
}

VOID Print_Network_Response_Data_Nodes() {
	if (Network_Response_Start_Node_Address == NULL)
		return;

	NetworkResponseData current = Network_Response_Start_Node_Address;
	while (current) {


		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "등록된 -> %s\n", (PCHAR)current->Data );


		current = (NetworkResponseData)current->Next_Addr;
	}

	return;
}


//
///
BOOLEAN Get_All_Response_list_Network_Response_Data__with_alloc(PDynamicData* output_start_buffer, PDynamicData* output_current_buffer) { // 할당 과 데이터 반환

	if (Network_Response_Start_Node_Address == NULL || output_start_buffer == NULL || output_current_buffer == NULL)
		return FALSE;

	K_object_init_check_also_lock_ifyouwant(&mutex_for_a_networkresponse, TRUE); // 상호배제


	// 시작점에 식별 문자열 넣기
	CHAR unq[] = "network";
	*output_start_buffer = CreateDynamicData((PUCHAR)&unq, (ULONG32)strlen((PCHAR)unq)); // 1. 

	PDynamicData current_output_buffer = *output_start_buffer;

	NetworkResponseData current = Network_Response_Start_Node_Address;
	while (current) {


		current_output_buffer = AppendDynamicData(current_output_buffer, current->Data, (ULONG32)strlen((PCHAR)current->Data) ); // 2.


		current = (NetworkResponseData)current->Next_Addr;
	}

	current_output_buffer = AppendDynamicData(current_output_buffer, (PUCHAR)"end", sizeof("end") - 1); // 3

	*output_current_buffer = current_output_buffer;

	K_object_lock_Release(&mutex_for_a_networkresponse);
	return TRUE;
}