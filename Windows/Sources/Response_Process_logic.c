#include "Response_Process.h"

#include "KEVENT_or_KMUTEX_init.h"
K_EVENT_or_MUTEX_struct mutex_process_response = { NULL, K_MUTEX, FALSE };

ProcessResponseData Process_Response_Start_Node_Address = NULL;
ProcessResponseData Process_Response_Current_Node_Address = NULL;


#include "File_io.h"
BOOLEAN Check_Process_Response(HANDLE* pid) { // �����ϱ� �� ����
	if (Process_Response_Start_Node_Address == NULL || Process_Response_Current_Node_Address == NULL)
		return FALSE;

	// ���� ���� PID�� ���� ũ�⸦ ����Ѵ�.
	ULONG32 EXE_SIZE = 0;
	if (!get_file_size(NULL, pid, &EXE_SIZE))
		return FALSE;


	K_object_init_check_also_lock_ifyouwant(&mutex_process_response, TRUE); // ��ȣ����

	ProcessResponseData current = Process_Response_Start_Node_Address;
	while (current) {

		ProcessResponse_Struct* processresponse_data = (ProcessResponse_Struct * )current->Data;

		if (EXE_SIZE == processresponse_data->FILE_SIZE) {

			// SHA256 ���ϱ�
			PCHAR got_sha256[SHA256_String_Byte_Length] = { 0, };
			if (FILE_to_INFO(
				NULL,
				pid,
				NULL,
				NULL,
				(PCHAR)got_sha256
			)) {
				if (memcmp((PCHAR)got_sha256, (PCHAR)processresponse_data->SHA256, (SHA256_String_Byte_Length - 1)) == 0) {
					// ��ġ��
					//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [TEST]-Check_Process_Response-��ġ�� , PID: %llu \n", *pid);
					K_object_lock_Release(&mutex_process_response);
					return TRUE;
				}
			}

			
		}

		current = (ProcessResponseData)current->Next_Addr;
	}


	K_object_lock_Release(&mutex_process_response);
	return FALSE;
}

#include "query_process_information.h"
BOOLEAN Append_Process_Response_Data(PCHAR SHA256, ULONG32 FILE_SIZE, ProcessResponseData* opt_output_start_node) {// ���� ��� �߰� ( ��, �ߺ� ���� )
	K_object_init_check_also_lock_ifyouwant(&mutex_process_response, TRUE); // ��ȣ����
	// ����ü ����
	ProcessResponse_Struct processresponse_data = { 0, };
	RtlCopyMemory(processresponse_data.SHA256, SHA256, SHA256_String_Byte_Length - 1); // null ���� 
	processresponse_data.FILE_SIZE = FILE_SIZE;

	// ��� �߰�
	if (Process_Response_Start_Node_Address == NULL) {
		Process_Response_Start_Node_Address = CreateDynamicData((PUCHAR)&processresponse_data, sizeof(processresponse_data));
		Process_Response_Current_Node_Address = Process_Response_Start_Node_Address;
	}
	else {
		// �ߺ� ��� üũ
		if (!is_exist_Process_Response_Data(SHA256)) {
			Process_Response_Current_Node_Address = AppendDynamicData(Process_Response_Current_Node_Address, (PUCHAR)&processresponse_data, sizeof(processresponse_data));
		}
		else {
			K_object_lock_Release(&mutex_process_response);
			return FALSE;
		}
		
	}
	Print_Process_Response_Data_Nodes();
	/*
		[+Update] - ���� �������� ���μ������� ������ SHA256�� �������̸� ���� �����Ѵ�.
	*/
	Terminate_Sturct Req_Running_Process_Terminate = {
		TRUE,
		processresponse_data.FILE_SIZE,
		processresponse_data.SHA256
	};
	Query_Running_Processes__(&Req_Running_Process_Terminate);


	*opt_output_start_node = Process_Response_Start_Node_Address;
	K_object_lock_Release(&mutex_process_response);
	return TRUE;
} 

BOOLEAN Remove_Process_Reponse_Data(PCHAR SHA256, ProcessResponseData* opt_output_start_node) { // ���� ��� ���� ( 1���� )
	if (Process_Response_Start_Node_Address == NULL)
		return FALSE;

	K_object_init_check_also_lock_ifyouwant(&mutex_process_response, TRUE); // ��ȣ����

	ProcessResponseData remember_previous_node = NULL;
	ProcessResponseData current = Process_Response_Start_Node_Address;
	while (current) {

		ProcessResponse_Struct* processresponse_data = (ProcessResponse_Struct*)current->Data;

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���μ��� ��ϵ� ���� SHA256 %64s ��û SHA-> %64s\n", processresponse_data->SHA256 , SHA256);
		if (memcmp(processresponse_data->SHA256, SHA256, SHA256_String_Byte_Length - 1) == 0) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���μ��� ���� ���� ��ġ\n");
			ProcessResponseData tmp = NULL;

			// ��� ���� �ؾ��Ѵ�. 
			if (current == Process_Response_Start_Node_Address) {
				tmp = current;
				Process_Response_Start_Node_Address = (ProcessResponseData)current->Next_Addr;
				if (current->Next_Addr) {
					Process_Response_Current_Node_Address = Process_Response_Start_Node_Address;
				}
				else {
					Process_Response_Current_Node_Address = NULL;
				}
				
			}
			else if (current == Process_Response_Current_Node_Address) {
				tmp = current;
				if (remember_previous_node) {
					remember_previous_node->Next_Addr = NULL;
					Process_Response_Current_Node_Address = remember_previous_node;
				}
				else {
					Process_Response_Start_Node_Address = NULL;
					Process_Response_Current_Node_Address = NULL;
				}
				
			}
			else {
				tmp = current;
				if (remember_previous_node == Process_Response_Start_Node_Address) {
					remember_previous_node = (ProcessResponseData)current->Next_Addr;
					if (!current->Next_Addr) {
						Process_Response_Current_Node_Address = NULL;
					}
				}
				else if (!current->Next_Addr) {
					if (remember_previous_node) {
						remember_previous_node->Next_Addr = NULL;
					}
					Process_Response_Current_Node_Address = remember_previous_node;
				}
				else {
					if(remember_previous_node)
						remember_previous_node->Next_Addr = current->Next_Addr;
				}
				
			}
			// ��� ����
			ExFreePoolWithTag(tmp->Data, 'BUFF');
			ExFreePoolWithTag(tmp, 'NODE');
			
			if (opt_output_start_node)
				*opt_output_start_node = Process_Response_Start_Node_Address;
			K_object_lock_Release(&mutex_process_response);
			return TRUE;
		}
		remember_previous_node = current;
		current = (ProcessResponseData)current->Next_Addr;
	}

	K_object_lock_Release(&mutex_process_response);
	return FALSE;
}
VOID Print_Process_Response_Data_Nodes() {
	if (Process_Response_Start_Node_Address == NULL)
		return;


	ProcessResponseData current = Process_Response_Start_Node_Address;
	while (current) {

		ProcessResponse_Struct* processresponse_data = (ProcessResponse_Struct*)current->Data;


		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [processresponse_data]// SHA256:%64s , FILE_SIZE: %lu \n", processresponse_data->SHA256, processresponse_data->FILE_SIZE);

		current = (ProcessResponseData)current->Next_Addr;
	}

	return;
}

BOOLEAN is_exist_Process_Response_Data(PCHAR SHA256) { // ���� ��� ���� Ȯ�� ��
	if (Process_Response_Start_Node_Address == NULL)
		return FALSE;


	ProcessResponseData current = Process_Response_Start_Node_Address;
	while (current) {

		ProcessResponse_Struct* processresponse_data = (ProcessResponse_Struct*)current->Data;

		if (memcmp(processresponse_data->SHA256, SHA256, SHA256_String_Byte_Length - 1) == 0) {
			return TRUE;
		}


		current = (ProcessResponseData)current->Next_Addr;
	}


	return FALSE;

}
