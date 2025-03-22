#include "Get_All_Response.h"

#include "Analysis_enum.h"
#include "DynamicData_2_lengthBuffer.h"

#include "Response_Network.h"
#include "Response_File.h"
#include "Response_Process.h"

BOOLEAN Get_All_Response_datas(
	PUCHAR* out_send_data,
	ULONG32* out_send_data_size
) {

	BOOLEAN output = TRUE;
	
	// 프로세스 연결노드 조회
	PDynamicData output_PROCESS_start_node = NULL;
	PDynamicData output_PROCESS_current_node = NULL;
	Get_All_Response_list_Process_Response_Data__with_alloc(&output_PROCESS_start_node, &output_PROCESS_current_node);
	

	// 네트워크 연결노드 조회
	PDynamicData output_NETWORK_start_node = NULL;
	PDynamicData output_NETWORK_current_node = NULL;
	Get_All_Response_list_Network_Response_Data__with_alloc(&output_NETWORK_start_node, &output_NETWORK_current_node);

	// 파일 시스템 연결노드 조회
	PDynamicData output_FILE_start_node = NULL;
	PDynamicData output_FILE_current_node = NULL;
	Get_All_Response_list_File_Response_Data__with_alloc(&output_FILE_start_node, &output_FILE_current_node);


	// 서로 연결
	// 프로세스 -> 네트워크 -> 파일 시스템
	PDynamicData All_in_Linked = NULL;

	if (output_PROCESS_start_node) {
		All_in_Linked = output_PROCESS_start_node;

		if (output_NETWORK_start_node) {
			output_PROCESS_current_node->Next_Addr = (PUCHAR)output_NETWORK_start_node;

			output_NETWORK_current_node->Next_Addr = (PUCHAR)output_FILE_start_node;
		}
		else if (output_FILE_start_node) {
			output_PROCESS_current_node->Next_Addr = (PUCHAR)output_FILE_start_node;
		}
		
	}
	else if (output_NETWORK_start_node) {
		All_in_Linked = output_NETWORK_start_node;

		if (output_FILE_start_node)
			output_NETWORK_start_node->Next_Addr = (PUCHAR)output_FILE_start_node;
	}
	else {
		if (output_FILE_start_node)
			All_in_Linked = output_FILE_start_node;
	}
	
	Analysis_Command cmd = 0;
	if (All_in_Linked) {
		 cmd = SUCCESS;

	}
	else {
		cmd = FAIL;
	}

	Makeing_Analysis_TCP(cmd, All_in_Linked, out_send_data, out_send_data_size);



	// 노드 모두 해제
	RemoveALLDynamicData(All_in_Linked);

	return output;

}