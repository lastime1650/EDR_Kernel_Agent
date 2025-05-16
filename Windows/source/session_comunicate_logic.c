#include "session_comunicate.h"

/*
	�ʱ� ����� �̷���� ��, 

	EDR ������ ����� ���������� �����Ѵ�.
*/

#include "util_Delay.h"
#include "Analysis_enum.h"

#include "TCP_send_or_Receiver.h"

#include "lengthBuffer_2_DynamicData.h"



#include "PROCESS_Request_ALL_Monitored_Data.h"
#include "PROCESS_Request_Real_File.h"

#include "PROCESS_Request_Process_Response.h"
#include "PROCESS_Request_Network_Response.h"
#include "PROCESS_Request_File_Response.h"

#include "Get_All_Response.h"
VOID Session_Communication(K_EVENT_or_MUTEX_struct* locked_Event) {
	Delays(-1);


	PUCHAR Received_Data = NULL;
	ULONG32 Received_Data_Size = 0;

	while (1) {

		/*
			���� -> �����ͼ��� -> ������ �Ľ� -> ��������� -> �м��������� -> ��������
		*/


		// (socket����->�����ͼ���)
		if (RECEIVE_TCP_DATA__with_alloc(&Received_Data, &Received_Data_Size, SERVER_DATA_PROCESS) != STATUS_SUCCESS) {
			if (Received_Data) {
				RECEIVE_or_SEND_TCP_DATA__with_alloc_Free(&Received_Data);
			}
			K_object_lock_Release(locked_Event); // ������ ��� ����
			return;
		}
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "1. RECEIVE_TCP_DATA__with_alloc ����\n");
		// (���ŵ� ���� -> �Ľ�(���Ḯ��Ʈ)
		Analysis_Command cmd;
		PDynamicData parsed_recv_data = NULL;
		lenBuffer_2_DynamicData(
			Received_Data,
			Received_Data_Size,
			(ULONG32*)&cmd,
			&parsed_recv_data
		);
		if (parsed_recv_data == NULL) {
			if (Received_Data) {
				RECEIVE_or_SEND_TCP_DATA__with_alloc_Free(&Received_Data);
			}
			K_object_lock_Release(locked_Event); // ������ ��� ����
			return;
		}
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "2. lenBuffer_2_DynamicData ����\n");
		// (����ؼ�) -> (ó����� �����)
		// ������ ������
		PUCHAR Send_Data = NULL;
		ULONG32 Send_Data_Size = 0;
		switch (cmd) {
		case Request_ALL_Monitored_Data:
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "3. Request_ALL_Monitored_Data\n");
			/*
				���ݱ��� ���� �����͸� �м��������� �����϶�
			*/
			PROCESS_Request_ALL_Monitored_Data(&Send_Data, &Send_Data_Size); // NULL�� �� ����
			break;
		}
		case Request_Real_File:
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "3. Request_Real_File\n");
			/*
				[0]: null�� ���� ���ϰ�� ( ANSI )
			*/
			PROCESS_Request_Real_File(parsed_recv_data,&Send_Data,&Send_Data_Size);; // NULL�� �� ����
			break;
		}
		case Response_Process:
		{
			/*
				[���μ���] ����
				[0]: SHA256 ( ANSI )
				[1]: FILE_SIZE (ULONG32) 
			*/
			PROCESS_Request_Process_Response(parsed_recv_data, &Send_Data, &Send_Data_Size, FALSE);
			break;
		}
		case Response_Process_Remove:
		{
			/*
				[���μ���] ���� ��� ��� ���� ( �����ڴ� SHA256 )
				[0]: SHA256 ( ANSI ) // �̰� �ϳ���
			*/
			PROCESS_Request_Process_Response(parsed_recv_data, &Send_Data, &Send_Data_Size, TRUE);
			break;
		}
		case Response_Network:
		{
			/*
				[��Ʈ��ũ] ����
				[0]: RemoteIP ( ANSI )
			*/
			PROCESS_Request_Network_Response(parsed_recv_data, &Send_Data, &Send_Data_Size, FALSE);
			break;
		}
		case Response_Network_Remove:
		{
			/*
				[��Ʈ��ũ] ���� ��� �ϳ� ����
				[0]: RemoteIP ( ANSI )
			*/
			PROCESS_Request_Network_Response(parsed_recv_data, &Send_Data, &Send_Data_Size, TRUE);
			break;
		}
		case Response_File:
		{
			/*
				[����] ����
				[0]: SHA256 ( ANSI )
				[1]: FILE_SIZE ( ULONG32 )
			*/
			PROCESS_Request_File_Response(parsed_recv_data, &Send_Data, &Send_Data_Size,FALSE);
			break;
		}
		case Response_File_Remove:
		{
			/*
				[����] ���� ��� ��� ���� ( �����ڴ� SHA256 )
				[0]: SHA256 ( ANSI ) // �̰� �ϳ���
			*/
			PROCESS_Request_File_Response(parsed_recv_data, &Send_Data, &Send_Data_Size,TRUE);
			break;
		}
		case get_Response_list:
		{
			/* ���� ���� �׳� ����/���μ���/��Ʈ��ũ ���� ������ ��� ��ȸ�Ͽ� ��ȯ�϶�. */
			Get_All_Response_datas(&Send_Data, &Send_Data_Size);
			break;
		}
		default:
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "3. ��ɽ���\n");
			// ����ó��
			if (Received_Data) {
				RECEIVE_or_SEND_TCP_DATA__with_alloc_Free(&Received_Data);
			}
			K_object_lock_Release(locked_Event); // ������ ��� ����
			return;
		}
		}

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "4. ���� ����\n");
		// (���� ����)
		if (SEND_TCP_DATA(Send_Data, Send_Data_Size, SERVER_DATA_PROCESS) != STATUS_SUCCESS) {
			// ������
			if (Send_Data) {
				ExFreePoolWithTag(Send_Data, session_alloc_tag);
			}

			if (parsed_recv_data) {
				RemoveALLDynamicData(parsed_recv_data);
			}

			if (Received_Data) {
				RECEIVE_or_SEND_TCP_DATA__with_alloc_Free(&Received_Data);
			}
			K_object_lock_Release(locked_Event); // ������ ��� ����
			return;
		}


		// ������
		if (Send_Data) {
			ExFreePoolWithTag(Send_Data, session_alloc_tag);
			Send_Data = NULL;
		}

		if (parsed_recv_data) {
			RemoveALLDynamicData(parsed_recv_data);
			parsed_recv_data = NULL;
		}

		if (Received_Data) {
			RECEIVE_or_SEND_TCP_DATA__with_alloc_Free(&Received_Data);
			Received_Data = NULL;
		}
		continue;
	}

}