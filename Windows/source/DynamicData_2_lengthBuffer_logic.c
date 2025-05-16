#include "DynamicData_2_lengthBuffer.h"


#include "Get_Time.h"

PUCHAR SEND_DATA = NULL;
ULONG32 SEND_DATA_SIZE = 0;

#include "KEVENT_or_KMUTEX_init.h"
K_EVENT_or_MUTEX_struct send_data_mutex = {NULL, K_MUTEX, FALSE};



// ����
VOID Output_lenBuff(PUCHAR* output_buffer, ULONG32* output_size) {
	if (!output_buffer || !output_size)
		return;

	K_object_init_check_also_lock_ifyouwant(&send_data_mutex, TRUE);

	// NULLüũ
	if (!SEND_DATA) {
		// null
		goto EXIT_1;
	}
	else {
		// null�ƴ�
		*output_buffer = ExAllocatePoolWithTag(PagedPool, SEND_DATA_SIZE, session_alloc_tag);
		if (!*output_buffer) {
			*output_buffer = NULL;
			*output_size = 0;
			goto EXIT;
		}

		RtlCopyMemory(*output_buffer, SEND_DATA, SEND_DATA_SIZE);
		*output_size = SEND_DATA_SIZE;
		
		goto EXIT_2;
	}


EXIT_2:
	{
		if (SEND_DATA) {
			ExFreePoolWithTag(SEND_DATA, LEN_BUFF_TAG);
		}
		goto EXIT_1;
	}
EXIT_1:
	{
		SEND_DATA = NULL;
		SEND_DATA_SIZE = 0;

		goto EXIT;
	}
EXIT:
	{
		K_object_lock_Release(&send_data_mutex);
		return;
	}
}

#include "converter_string.h"
#include "File_io.h"
#include "is_system_pid.h"
#include "query_process_information.h"
#include "SHA256.h"
#include "util_Delay.h"
ULONG32 append_size(ULONG32 size);
ULONG32 append_Node_pid_path(PDynamicData current, PDynamicData* output_current, HANDLE input_pid); // �� ��忡 PID�� ������ �����͸� �߰� �Ѵ�. 
VOID Dyn_2_lenBuff(Pmover in_parameter) {

	if (!in_parameter->start_node)
		return;

	PUCHAR DATA = NULL;
	ULONG32 SIZE = 0;



	// �� ����� �ؼ��Ͽ� �������� ���� "���̱�� �ѱ���"�� *����*���Ѵ� 
	switch (in_parameter->cmd) {
	case CmRegisterCallbackEx_for_mon:
	{
		PDynamicData current = in_parameter->start_node;

		/*������*/
		PUCHAR key = (PUCHAR)current->Data;																			// 1.
		SIZE += append_size( current->Size);
		current = (PDynamicData)current->Next_Addr;

		PCHAR keyobject = (PCHAR)current->Data;																		// 2.
		SIZE += append_size(current->Size);

		if (
			strncmp(
				(PCHAR)key,
				"RegSetValueKey",
				sizeof("RegSetValueKey") - 1
			) == 0
			) {
			// ValueName ����
			current = (PDynamicData)current->Next_Addr;
			if (current) {
				SIZE += append_size(current->Size);

				// Type-ulong ����
				current = (PDynamicData)current->Next_Addr;
				SIZE += append_size(current->Size);

				// ���� �����ʹ� ���� Next_Addr���� NULL�̸� ���� ��
				current = (PDynamicData)current->Next_Addr;
				if (current)
					SIZE += append_size(current->Size);
			}
			
		}
		else if (
			strncmp(
				(PCHAR)key,
				"RegDeleteValueKey",
				sizeof("RegDeleteValueKey") - 1
			) == 0
			) {
			// ValueName ����
			current = (PDynamicData)current->Next_Addr;
			if(current)
				SIZE += append_size(current->Size);
		}
		else if (
			strncmp(
				(PCHAR)key,
				"RegRenameKey",
				sizeof("RegRenameKey") - 1
			) == 0
			) {
			// Rename�� NewValueName ����
			current = (PDynamicData)current->Next_Addr;
			if (current)
				SIZE += append_size(current->Size);
		}


		//SIZE += append_Node_pid_path(current, &current, in_parameter->PID); // 3.

		//keyobject;
		

		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "2222 CmRegisterCallbackEx_for_mon -> %p \n", in_parameter->start_node);

		key; keyobject;
		break;
	}
	case PsSetCreateProcessNotifyRoutine_Creation_Detail:
	{

		PDynamicData current = in_parameter->start_node;

		/* ������ */
		HANDLE ParentID = *(HANDLE*)current->Data;													// 1. 
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		HANDLE ThreadID = *(HANDLE*)current->Data;													// 2.
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		PCHAR CommandLine = (PCHAR)current->Data;													// 3.
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;


		PCHAR EXE_name = (PCHAR)current->Data;	EXE_name;												// 
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		PCHAR SHA256 = (PCHAR)current->Data; SHA256;													// 
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		PUCHAR exe_size = (PUCHAR)current->Data; exe_size;
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;


		PCHAR Parent_ImageName = (PCHAR)current->Data;	Parent_ImageName;												// 
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		PCHAR SHA256_2 = (PCHAR)current->Data; SHA256_2;													// 
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		PUCHAR exe_size2 = (PUCHAR)current->Data; exe_size2;
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		


		ParentID; ThreadID; CommandLine; //EXE_NAME; /* SHA256 , FILE_SIZE */
		break;
	}
	case PsSetCreateProcessNotifyRoutine_Remove:
	{
		PDynamicData current = in_parameter->start_node;

		/* ������ */
		HANDLE ParentID = *(HANDLE*)current->Data; // 1.
		SIZE += append_size(current->Size);

		ParentID;
		break;
	}
	case PsSetLoadImageNotifyRoutine_Load:
	{
		PDynamicData current = in_parameter->start_node;

		/*������*/
		PCHAR ImageNameA = (PCHAR)current->Data;
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		SIZE_T ImageSize = *(PSIZE_T)current->Data;													// 2.
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		PUCHAR sha256 = current->Data;
		SIZE += append_size(current->Size);

		ImageSize; ImageSize; sha256; ImageNameA;
		break;
	}
	case File_System:
	{
		/*
			�ý��� EXE �˻縦 ���⼭ �����Ѵ�. ( ���Ⱑ ������ PASSIVE �������� )
		*/
		if (Is_it_System_Process(in_parameter->PID)) {
			goto INIT_FAILED;
		}


		PDynamicData current = in_parameter->start_node;

		/*������*/
		PWCH File_Path_WCH = (PWCH)current->Data;																 // 1.

		UNICODE_STRING File_Path_W = { 0, };
		RtlInitUnicodeString(&File_Path_W, File_Path_WCH);

		ANSI_STRING File_Path_A = { 0, };
		UNICODE_to_ANSI(&File_Path_A, &File_Path_W);
		//ANSI_STRING File_Path = { 0, };
		//PWCH_to_ANSI(&File_Path, File_Path_W);
		Change_Node_Data(current, (PUCHAR)File_Path_A.Buffer, File_Path_A.MaximumLength - 1);
		UNICODE_to_ANSI_release(&File_Path_A);
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		PULONG32 File_SIze = (PULONG32)current->Data;															// 2. ���� ����
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;
		File_SIze;

		PCHAR DENIED_CHECK = (PCHAR)current->Data;	DENIED_CHECK;														// 3.
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		PCHAR File_Behavior = (PCHAR)current->Data;															// 4.
		SIZE += append_size(current->Size);
		
		

		/* File_Behavior�� ���� ��� ���� '����'�� */
		// [1] Rename�� ��尡 1�� �߰��� 
		if (memcmp(File_Behavior, "Rename", sizeof("Rename") - 1) == 0) {
			current = (PDynamicData)current->Next_Addr;
			if (current) {
				PWCH Rename_File_Path_W = (PWCH)current->Data;												// 5.
				ANSI_STRING Rename_File_Path = { 0, };
				PWCH_to_ANSI(&Rename_File_Path, Rename_File_Path_W);
				Change_Node_Data(current, (PUCHAR)Rename_File_Path.Buffer, Rename_File_Path.MaximumLength - 1);
				PWCH_to_ANSI_release(&Rename_File_Path);
				SIZE += append_size(current->Size);
				
			}
		}
		//current = (PDynamicData)current->Next_Addr;

		// ���� ������ ( ��� ��� )																		// 4. 
		/*
		ULONG32 File_Size = 0;
		if (get_file_size(&File_Path_W, NULL, &File_Size, FALSE)) {
			SIZE += append_size( sizeof(File_Size) );
			current = AppendDynamicData(current, (PUCHAR)&File_Size, sizeof(File_Size));
		}
		*/
		
		

		//* ���� ���̳ʸ� �������� �õ� ( ���� �뷮 ������ �ɾ�� �ϰų� COre-server ���� ��û�ϰų�? ) *//


		//File_Path;  File_Behavior; File_SIze; // ( Rename�� ��� "FILE_RENAME_FILE_PATH" ��� �߰��Ͽ� �� 4���̻�
		// Rename_File_Path_W
		break;
	}
	
	case NDIS_Network_Traffic:
	{
		/*
			�ý��� EXE �˻縦 ���⼭ �����Ѵ�. ( ���Ⱑ ������ PASSIVE �������� ) - work�� ����
		*/
		if (Is_it_System_Process(in_parameter->PID)) {
			goto INIT_FAILED;
		}

		PDynamicData current = in_parameter->start_node;

		/* ������ */
		ULONG32 protocol = *(ULONG32*)current->Data;
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		ULONG32 is_INBOUND = *(ULONG32*)current->Data;
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;
		
		ULONG32 packetSize = *(ULONG32*)current->Data;
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;
		/*
		PUCHAR Packet_BUffer = current->Data;
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;
		*/
		PUCHAR LOCAL_IPv4_STR = current->Data; LOCAL_IPv4_STR;
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		ULONG32 LOCAL_PORT = *(ULONG32*)current->Data; LOCAL_PORT;
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;
		
		PUCHAR REMOTE_IPv4_STR = current->Data;
		SIZE += append_size(current->Size);
		current = (PDynamicData)current->Next_Addr;

		ULONG32 REMOTE_PORT = *(ULONG32*)current->Data;
		SIZE += append_size(current->Size);

		//SIZE += append_Node_pid_path(current, &current, in_parameter->PID); // ....

		protocol; is_INBOUND; packetSize; REMOTE_IPv4_STR; REMOTE_PORT; // Packet_BUffer; LOCAL_IPv4_STR; LOCAL_PORT;
		break;
	}
	default:
		goto EXIT;
	}


	// �ʹ� ���� ���ϱ�
	SIZE += Head;// sizeof(in_parameter->cmd);					// ���
	SIZE += LENGTH_MAX_BYTE_NUM + sizeof(in_parameter->PID);	// PID
	SIZE += LENGTH_MAX_BYTE_NUM + Time_Length() - 1;			// Ÿ�ӽ����� null���� ����
	SIZE += sizeof(Tail) - 1;

	// �����Ҵ� �õ� ( ��뷮 �Ҵ� ���� �߻� ) 
	DATA = (PUCHAR)ExAllocatePoolWithTag(PagedPool, SIZE, LEN_BUFF_TAG);
	if (!DATA)
		goto EXIT;
	memset(DATA, 0, SIZE);

	/* ���Խõ� */

	PUCHAR current_addr = DATA;

	// 1. cmd -> pid -> timestamp �κ�
	{
		//// cmd
		RtlCopyMemory(current_addr, (PUCHAR)&in_parameter->cmd, sizeof(in_parameter->cmd));
		current_addr = current_addr + sizeof(in_parameter->cmd);

		//// pid 
		// [1/2] ���� ���� part
		ULONG32 pid_size = sizeof(in_parameter->PID);
		RtlCopyMemory(current_addr, (PUCHAR)&pid_size, sizeof(pid_size));
		current_addr = current_addr + LENGTH_MAX_BYTE_NUM;

		// [2/2] ���� ���� ������
		RtlCopyMemory(current_addr, (PUCHAR)&in_parameter->PID, sizeof(in_parameter->PID));
		current_addr = current_addr + sizeof(in_parameter->PID);


		//// timestamp
		ULONG32 timestamp_len = Time_Length() - 1;
		RtlCopyMemory(current_addr, (PUCHAR)&timestamp_len, sizeof(timestamp_len));
		current_addr = current_addr + LENGTH_MAX_BYTE_NUM;

		// [2/2] ���� ���� ������
		RtlCopyMemory(current_addr, (PUCHAR)in_parameter->timestamp, timestamp_len);
		current_addr = current_addr + timestamp_len;
		Release_Got_Time(in_parameter->timestamp);

	}



	{
		// 2. while �ݺ����� �������� ����
		PDynamicData tmp = in_parameter->start_node;
		while (tmp != NULL) {

			// [1/2] ���� ���� part
			RtlCopyMemory(current_addr, (PUCHAR)&tmp->Size, sizeof(tmp->Size));
			current_addr = current_addr + LENGTH_MAX_BYTE_NUM;

			// [2/2] ���� ���� ������
			RtlCopyMemory(current_addr, tmp->Data, tmp->Size);
			current_addr = current_addr + tmp->Size;

			tmp = (PDynamicData)tmp->Next_Addr;
			continue;
		}
		RemoveALLDynamicData(in_parameter->start_node);
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "33333 CmRegisterCallbackEx_for_mon -> %p \n", in_parameter->start_node);
		
	}


	{
		// 3. "END_"�߰�
		RtlCopyMemory(current_addr, (PUCHAR)Tail, sizeof(Tail) - 1);
	}










	{

		/* ������ ������ ���������� ���� */
		K_object_init_check_also_lock_ifyouwant(&send_data_mutex, TRUE);

		if (SEND_DATA && SEND_DATA_SIZE > 999999) {
			ExFreePoolWithTag(SEND_DATA, LEN_BUFF_TAG);
			SEND_DATA = NULL;
			SEND_DATA_SIZE = 0;
		}

		if (SEND_DATA == NULL) {
			// ��� �Ҵ��� ���۸� �״�� �����Ѵ�.
			SEND_DATA = DATA;
			SEND_DATA_SIZE = SIZE;
			goto EXIT;
		}
		else {
			// update �Ѵ� ( �̾ �����ϱ� ) 

			PUCHAR update_data = NULL;
			ULONG32 update_data_size = SEND_DATA_SIZE + SIZE;

			update_data = ExAllocatePoolWithTag(PagedPool, update_data_size, LEN_BUFF_TAG);
			if (!update_data)
				goto EXIT_1;

			// [1/2]
			RtlCopyMemory(update_data, SEND_DATA, SEND_DATA_SIZE);

			// [2/2]
			RtlCopyMemory(update_data + SEND_DATA_SIZE, DATA, SIZE);


			ExFreePoolWithTag(SEND_DATA, LEN_BUFF_TAG);
			SEND_DATA = NULL;
			SEND_DATA_SIZE = 0;

			// final ������Ʈ
			SEND_DATA = update_data;
			SEND_DATA_SIZE = update_data_size;

		}
		
	}
	
	goto EXIT_1;

EXIT_1:
	{
		if (DATA && (SIZE > 0) )
			ExFreePoolWithTag(DATA, LEN_BUFF_TAG);
		goto EXIT;
	}
EXIT:
	{
		ExFreePoolWithTag(in_parameter, mover_tag);
		K_object_lock_Release(&send_data_mutex);
		return;
	}
	//
INIT_FAILED: // �������� ó���� �� ������ ��쿡�� �̷� goto �϶� 
	{
		RemoveALLDynamicData(in_parameter->start_node);
		Release_Got_Time(in_parameter->timestamp);
		ExFreePoolWithTag(in_parameter, mover_tag);
		return;
	}
}

ULONG32 append_size(
	ULONG32 size
) {
	return LENGTH_MAX_BYTE_NUM + size;
}
#include "util_Delay.h"
ULONG32 append_Node_pid_path(PDynamicData current, PDynamicData* output_current, HANDLE input_pid) {
	if (!current)
		return 0;
	
	ULONG32 outputSIZE = 0;

	// PID ���� EXE ������ ���
	PUNICODE_STRING exe_file_path = NULL;
	if (Query_Process_info(input_pid, ProcessImageFileName, &exe_file_path) == STATUS_SUCCESS) {

		////////////////////////////////////////////////////////////////////////////////////////////// 1. PID ���� ������ �߰�
		ANSI_STRING PID_FILE_PATH_A = { 0, };
		PWCH_to_ANSI(&PID_FILE_PATH_A, exe_file_path->Buffer);
		current = AppendDynamicData(current, (PUCHAR)PID_FILE_PATH_A.Buffer, (PID_FILE_PATH_A.MaximumLength - 1)); // PID 
		PWCH_to_ANSI_release(&PID_FILE_PATH_A);
		outputSIZE += append_size(current->Size);

		ULONG32 exe_size = 0;
		CHAR SHA256[SHA256_String_Byte_Length] = { 0, };
		if (!FILE_to_INFO(
			exe_file_path,
			NULL,
			&exe_size,
			NULL,
			SHA256
		)) {
			Query_Process_Image_Name___Release_Free_POOL(exe_file_path);
			*output_current = current;
			return outputSIZE;
		}
		else {
			

			//////////////////////////////////////////////////////////////////////////////////////// 2. SHA256  �߰�
			current = AppendDynamicData(current, (PUCHAR)SHA256, (SHA256_String_Byte_Length - 1)); //  SHA256
			//PCHAR SHA256 = current->Data;
			outputSIZE += append_size(current->Size);

			Query_Process_Image_Name___Release_Free_POOL(exe_file_path); // PID ������ �Ҵ�����

			/////////////////////////////////////////////////////////////////////////////////////// 3. ���� ũ�� �߰�
			current = AppendDynamicData(current, (PUCHAR)&exe_size, sizeof(exe_size));
			outputSIZE += append_size(current->Size);


			*output_current = current;
			//SHA256;
		}


	}

	return outputSIZE;
}


//

#include "session_comunicate.h"
BOOLEAN Makeing_Analysis_TCP(ULONG32 Command, PDynamicData Input_Dynamic_Data, PUCHAR* OUTPUT, ULONG32* OUTPUT_size) {

	if (!OUTPUT || !OUTPUT_size) return FALSE;

	// ���� ��� ���̸� ����ؾ��Ѵ�. 
	ULONG32 Dynamic_data_size = 0;
	ULONG32 Dynamic_node_count = GetDynamicDataNodeCount(Input_Dynamic_Data, &Dynamic_data_size);

	*OUTPUT_size =
		//sizeof(Topic) + (���ŵ� 2024/12/14)
		sizeof(Command) +

		(sizeof(ULONG32) * Dynamic_node_count) +
		Dynamic_data_size +

		(sizeof(ULONG32) + Time_Length() - 1) + // �ð����� 

		sizeof("_END") - 1
		;
	*OUTPUT = ExAllocatePoolWithTag(PagedPool, *OUTPUT_size, session_alloc_tag);
	if (!*OUTPUT) return FALSE;

	ULONG32 current_offset = 0;
	//RtlCopyMemory(*OUTPUT + current_offset, &Topic, sizeof(Topic)); (���ŵ� 2024/12/14)
	//current_offset += sizeof(Topic); (���ŵ� 2024/12/14)

	RtlCopyMemory(*OUTPUT + current_offset, &Command, sizeof(Command));
	current_offset += sizeof(Command);

	PDynamicData current = Input_Dynamic_Data;
	while (current != NULL) {
		ULONG32 current_size = current->Size;

		RtlCopyMemory(*OUTPUT + current_offset, &current_size, sizeof(current_size));
		current_offset += sizeof(current_size);

		RtlCopyMemory(*OUTPUT + current_offset, current->Data, current_size);
		current_offset += current_size;

		current = (PDynamicData)current->Next_Addr;
	}

	// ������+�ð� Set1 �߰�
	ULONG32 time_size = Time_Length() - 1;
	RtlCopyMemory(*OUTPUT + current_offset, &time_size, sizeof(ULONG32));
	current_offset += sizeof(ULONG32);

	PCHAR Time = Get_Time();
	RtlCopyMemory(*OUTPUT + current_offset, Time, Time_Length() - 1);

	current_offset += Time_Length() - 1;

	Release_Got_Time(Time);

	// _END �߰�
	RtlCopyMemory(*OUTPUT + current_offset, "_END", sizeof("_END") - 1);


	return TRUE;
}