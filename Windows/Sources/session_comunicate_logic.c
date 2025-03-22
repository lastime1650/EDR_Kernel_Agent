#include "session_comunicate.h"

/*
	초기 통신이 이루어진 후, 

	EDR 서버와 통신을 지속적으로 유지한다.
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
			점유 -> 데이터수신 -> 데이터 파싱 -> 결과가져옴 -> 분석서버전달 -> 점유해제
		*/


		// (socket점유->데이터수신)
		if (RECEIVE_TCP_DATA__with_alloc(&Received_Data, &Received_Data_Size, SERVER_DATA_PROCESS) != STATUS_SUCCESS) {
			if (Received_Data) {
				RECEIVE_or_SEND_TCP_DATA__with_alloc_Free(&Received_Data);
			}
			K_object_lock_Release(locked_Event); // 스레드 잠금 해제
			return;
		}
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "1. RECEIVE_TCP_DATA__with_alloc 성공\n");
		// (수신된 버퍼 -> 파싱(연결리스트)
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
			K_object_lock_Release(locked_Event); // 스레드 잠금 해제
			return;
		}
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "2. lenBuffer_2_DynamicData 성공\n");
		// (명령해석) -> (처리결과 만들기)
		// 전달할 데이터
		PUCHAR Send_Data = NULL;
		ULONG32 Send_Data_Size = 0;
		switch (cmd) {
		case Request_ALL_Monitored_Data:
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "3. Request_ALL_Monitored_Data\n");
			/*
				지금까지 모은 데이터를 분석서버에게 제공하라
			*/
			PROCESS_Request_ALL_Monitored_Data(&Send_Data, &Send_Data_Size); // NULL일 수 없음
			break;
		}
		case Request_Real_File:
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "3. Request_Real_File\n");
			/*
				[0]: null이 없는 파일경로 ( ANSI )
			*/
			PROCESS_Request_Real_File(parsed_recv_data,&Send_Data,&Send_Data_Size);; // NULL일 수 없음
			break;
		}
		case Response_Process:
		{
			/*
				[프로세스] 차단
				[0]: SHA256 ( ANSI )
				[1]: FILE_SIZE (ULONG32) 
			*/
			PROCESS_Request_Process_Response(parsed_recv_data, &Send_Data, &Send_Data_Size, FALSE);
			break;
		}
		case Response_Process_Remove:
		{
			/*
				[프로세스] 차단 등록 노드 제거 ( 기준자는 SHA256 )
				[0]: SHA256 ( ANSI ) // 이거 하나만
			*/
			PROCESS_Request_Process_Response(parsed_recv_data, &Send_Data, &Send_Data_Size, TRUE);
			break;
		}
		case Response_Network:
		{
			/*
				[네트워크] 차단
				[0]: RemoteIP ( ANSI )
			*/
			PROCESS_Request_Network_Response(parsed_recv_data, &Send_Data, &Send_Data_Size, FALSE);
			break;
		}
		case Response_Network_Remove:
		{
			/*
				[네트워크] 차단 노드 하나 해제
				[0]: RemoteIP ( ANSI )
			*/
			PROCESS_Request_Network_Response(parsed_recv_data, &Send_Data, &Send_Data_Size, TRUE);
			break;
		}
		case Response_File:
		{
			/*
				[파일] 차단
				[0]: SHA256 ( ANSI )
				[1]: FILE_SIZE ( ULONG32 )
			*/
			PROCESS_Request_File_Response(parsed_recv_data, &Send_Data, &Send_Data_Size,FALSE);
			break;
		}
		case Response_File_Remove:
		{
			/*
				[파일] 차단 등록 노드 제거 ( 기준자는 SHA256 )
				[0]: SHA256 ( ANSI ) // 이거 하나만
			*/
			PROCESS_Request_File_Response(parsed_recv_data, &Send_Data, &Send_Data_Size,TRUE);
			break;
		}
		case get_Response_list:
		{
			/* 인자 없음 그냥 파일/프로세스/네트워크 차단 연결노드 모두 순회하여 반환하라. */
			Get_All_Response_datas(&Send_Data, &Send_Data_Size);
			break;
		}
		default:
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "3. 명령실패\n");
			// 실패처리
			if (Received_Data) {
				RECEIVE_or_SEND_TCP_DATA__with_alloc_Free(&Received_Data);
			}
			K_object_lock_Release(locked_Event); // 스레드 잠금 해제
			return;
		}
		}

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "4. 서버 전달\n");
		// (서버 전달)
		if (SEND_TCP_DATA(Send_Data, Send_Data_Size, SERVER_DATA_PROCESS) != STATUS_SUCCESS) {
			// 마무리
			if (Send_Data) {
				ExFreePoolWithTag(Send_Data, session_alloc_tag);
			}

			if (parsed_recv_data) {
				RemoveALLDynamicData(parsed_recv_data);
			}

			if (Received_Data) {
				RECEIVE_or_SEND_TCP_DATA__with_alloc_Free(&Received_Data);
			}
			K_object_lock_Release(locked_Event); // 스레드 잠금 해제
			return;
		}


		// 마무리
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