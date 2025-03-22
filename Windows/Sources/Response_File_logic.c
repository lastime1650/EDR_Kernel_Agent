#include "Response_File.h"
#include "KEVENT_or_KMUTEX_init.h"
K_EVENT_or_MUTEX_struct mutex_responsefile = { NULL,K_MUTEX,FALSE };

FileResponseData File_Response_Start_Node_Address = NULL;
FileResponseData File_Response_Current_Node_Address = NULL;

#include "File_io.h"
BOOLEAN is_same_check(PUNICODE_STRING target_FilePATH, ULONG32 target_File_SIze) { // ����( ���� ���� Ȯ�� �� -> ���� )
	if (File_Response_Start_Node_Address == NULL && KeGetCurrentIrql() != PASSIVE_LEVEL)
		return FALSE;
	K_object_init_check_also_lock_ifyouwant(&mutex_responsefile, TRUE); // ��ȣ����
	FileResponseData current = File_Response_Start_Node_Address;
	while (current) {

		FileResponse_Struct* fileresponse_data = (FileResponse_Struct*)current->Data;

		if (fileresponse_data->File_Size == target_File_SIze) {

			// SHA256 ���ϱ�
			PCHAR got_sha256[SHA256_String_Byte_Length] = { 0, };
			if (FILE_to_INFO(
				target_FilePATH,
				NULL,
				NULL,
				NULL,
				(PCHAR)got_sha256
			)) {
				if (memcmp((PCHAR)got_sha256, (PCHAR)fileresponse_data->SHA256, (SHA256_String_Byte_Length - 1)) == 0) {
					// ��ġ��
					K_object_lock_Release(&mutex_responsefile);
					return TRUE;
				}
			}

		}

		current = (FileResponseData)current->Next_Addr;
	}
	K_object_lock_Release(&mutex_responsefile);
	return FALSE;
}

BOOLEAN Append_File_Response_Data(PCHAR SHA256, ULONG32 File_SIze, FileResponseData* opt_output_start_node) { // ���� ��� �߰� ( ��, �ߺ� ���� )

	if (File_SIze > 500000000) // 500MB ����
		return FALSE;

	K_object_init_check_also_lock_ifyouwant(&mutex_responsefile, TRUE);

	FileResponse_Struct file_struct = {0,};
	RtlCopyMemory(file_struct.SHA256, SHA256, SHA256_String_Byte_Length - 1);
	file_struct.File_Size = File_SIze;

	if (File_Response_Start_Node_Address == NULL) {
		File_Response_Start_Node_Address = CreateDynamicData((PUCHAR)&file_struct, sizeof(file_struct));
		File_Response_Current_Node_Address = File_Response_Start_Node_Address;
	}
	else {
		if (is_exist_File_Response_Data(SHA256)) {
			K_object_lock_Release(&mutex_responsefile);
			return FALSE;
		}
		else {
			File_Response_Current_Node_Address = AppendDynamicData(File_Response_Current_Node_Address, (PUCHAR)&file_struct, sizeof(file_struct));
		}
	}
	
	if(opt_output_start_node)
		*opt_output_start_node = File_Response_Start_Node_Address;

	Print_File_Response_Data_Nodes();

	K_object_lock_Release(&mutex_responsefile);
	return TRUE;
}
BOOLEAN Remove_File_Reponse_Data(PCHAR SHA256, FileResponseData* opt_output_start_node) { // ���� ��� ���� ( 1���� )
	if (File_Response_Start_Node_Address == NULL)
		return FALSE;

	K_object_init_check_also_lock_ifyouwant(&mutex_responsefile, TRUE); // ��ȣ����

	FileResponseData remember_previous_node = NULL;
	FileResponseData current = File_Response_Start_Node_Address;
	while (current) {

		FileResponse_Struct* processresponse_data = (FileResponse_Struct*)current->Data;

		if (memcmp(processresponse_data->SHA256, SHA256, SHA256_String_Byte_Length - 1) == 0) {

			FileResponseData tmp = NULL;

			// ��� ���� �ؾ��Ѵ�. 
			if (current == File_Response_Start_Node_Address) {
				tmp = current;
				File_Response_Start_Node_Address = (FileResponseData)current->Next_Addr;
				if (current->Next_Addr) {
					File_Response_Current_Node_Address = File_Response_Start_Node_Address;
				}
				else {
					File_Response_Current_Node_Address = NULL;
				}

			}
			else if (current == File_Response_Current_Node_Address) {
				tmp = current;
				if (remember_previous_node) {
					remember_previous_node->Next_Addr = NULL;
					File_Response_Current_Node_Address = remember_previous_node;
				}
				else {
					File_Response_Start_Node_Address = NULL;
					File_Response_Current_Node_Address = NULL;
				}

			}
			else {
				tmp = current;
				if (remember_previous_node == File_Response_Start_Node_Address) {
					remember_previous_node = (FileResponseData)current->Next_Addr;
					if (!current->Next_Addr) {
						File_Response_Current_Node_Address = NULL;
					}
				}
				else if (!current->Next_Addr) {
					if (remember_previous_node) {
						remember_previous_node->Next_Addr = NULL;
					}
					File_Response_Current_Node_Address = remember_previous_node;
				}
				else {
					if (remember_previous_node)
						remember_previous_node->Next_Addr = current->Next_Addr;
				}

			}
			// ��� ����
			ExFreePoolWithTag(tmp->Data, 'BUFF');
			ExFreePoolWithTag(tmp, 'NODE');

			if (opt_output_start_node)
				*opt_output_start_node = File_Response_Start_Node_Address;
			K_object_lock_Release(&mutex_responsefile);
			return TRUE;
		}
		remember_previous_node = current;
		current = (FileResponseData)current->Next_Addr;
	}

	K_object_lock_Release(&mutex_responsefile);
	return FALSE;
}

BOOLEAN is_exist_File_Response_Data(PCHAR SHA256) { // ���� ��� Ȯ��
	if (File_Response_Start_Node_Address == NULL)
		return FALSE;


	FileResponseData current = File_Response_Start_Node_Address;
	while (current) {

		FileResponse_Struct* fileresponse_data = (FileResponse_Struct*)current->Data;

		if (memcmp((PCHAR)fileresponse_data->SHA256, SHA256, SHA256_String_Byte_Length - 1) == 0) {
			return TRUE;
		}


		current = (FileResponseData)current->Next_Addr;
	}


	return FALSE;


}

VOID Print_File_Response_Data_Nodes() {
	if (File_Response_Start_Node_Address == NULL)
		return;


	FileResponseData current = File_Response_Start_Node_Address;
	while (current) {

		FileResponse_Struct* fileresponse_data = (FileResponse_Struct*)current->Data;

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " ����  SHA256  -> %64s / ������ -> %lu \n", fileresponse_data->SHA256, fileresponse_data->File_Size);


		current = (FileResponseData)current->Next_Addr;
	}


	return;
}

///
BOOLEAN Get_All_Response_list_File_Response_Data__with_alloc(PDynamicData* output_start_buffer, PDynamicData* output_current_buffer) { // �Ҵ� �� ������ ��ȯ

	if (File_Response_Start_Node_Address == NULL || output_start_buffer == NULL || output_current_buffer == NULL)
		return FALSE;

	K_object_init_check_also_lock_ifyouwant(&mutex_responsefile, TRUE); // ��ȣ����


	// �������� �ĺ� ���ڿ� �ֱ�
	UCHAR unq[] = "file_system";
	*output_start_buffer = CreateDynamicData((PUCHAR)&unq, (ULONG32)strlen((PCHAR)unq)); // 1. 


	PDynamicData current_output_buffer = *output_start_buffer;

	FileResponseData current = File_Response_Start_Node_Address;
	while (current) {

		FileResponse_Struct* file_response_data = (FileResponse_Struct*)current->Data;


		current_output_buffer = AppendDynamicData(current_output_buffer, (PUCHAR)&file_response_data->SHA256, sizeof(file_response_data->SHA256)); // 2.
		current_output_buffer = AppendDynamicData(current_output_buffer, (PUCHAR)&file_response_data->File_Size, sizeof(file_response_data->File_Size)); // 3.


		current = (FileResponseData)current->Next_Addr;
	}

	current_output_buffer = AppendDynamicData(current_output_buffer, (PUCHAR)"end", sizeof("end")-1); // 4.

	*output_current_buffer = current_output_buffer;

	K_object_lock_Release(&mutex_responsefile);
	return TRUE;
}