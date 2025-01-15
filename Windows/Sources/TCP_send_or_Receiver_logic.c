#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "TCP_send_or_Receiver.h"
#include "util_Delay.h"


K_EVENT_or_MUTEX_struct TCP_session_MUTEX = { NULL, K_MUTEX, FALSE };

/*
	[송신]
		1. 점유 -> 송신 * 2 -> 점유해제 -> 반환
		2. 송신	* 2 -> 반환 

	[수신]
		1. 점유 -> 수신 * 2 -> 점유해제 -> 반환
		2. 점유 -> 수신 * 2 -> 반환

*/


NTSTATUS RECEIVE_TCP_DATA__with_alloc(PVOID* output_BUFFER, ULONG32* output_BUFFER_SIZE, SOCKET_INFORMATION receive_types) { // 수신할 때

	K_object_init_check_also_lock_ifyouwant(&TCP_session_MUTEX, FALSE); // 초기화는 하지만 점유시도는 안함

	/*
		총 2차 과정으로 Receive함
		[1/2]
			4바이트를 SERVER로부터 읽어서 정수로 변환하여 저장
		[2/2]
			([1/2]) 읽은 것만큼의 크기로 동적할당함
	*/
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc -> TCP 뮤텍스 점유시도 \n");
	K_object_init_check_also_lock_ifyouwant(&TCP_session_MUTEX, TRUE); // Lock
	NTSTATUS status = STATUS_SUCCESS;

	*output_BUFFER = NULL;
	ULONG32 allocate_size = 0;
	if (Send_or_Receive_TCP_Server(&allocate_size, sizeof(allocate_size), TCP_DATA_RECEIVE, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[1/2]
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 2 (실패)  \n");
		status = STATUS_UNSUCCESSFUL;
	}
	else {
		if (allocate_size == 0) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 2-1 (실패) allocate_size길이가 0 임  \n");
			status = STATUS_UNSUCCESSFUL;
		}

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 3 (1차 성공) 서버로부터 가져온 길이 -> %lu \n", allocate_size);
		*output_BUFFER_SIZE = allocate_size; // 동적크기
		*output_BUFFER = (PVOID)ExAllocatePoolWithTag(PagedPool, *output_BUFFER_SIZE, 'RcDt'); // 동적할당
		if (*output_BUFFER == NULL || allocate_size == 0) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 4 \n");
			status = STATUS_INSUFFICIENT_RESOURCES;
		}
		else {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 5 ( 2차를 대기하고 있습니다.. )  \n");
			//Delays(-1);
			if (Send_or_Receive_TCP_Server(*output_BUFFER, *output_BUFFER_SIZE, TCP_DATA_RECEIVE, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[2/2]
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 6 \n");
				status = STATUS_UNSUCCESSFUL;
			}
			else {
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 7 \n");
				status = STATUS_SUCCESS;
			}
		}
	}

	if (receive_types != SERVER_DATA_PROCESS || status != STATUS_SUCCESS) {
		K_object_lock_Release(&TCP_session_MUTEX); // Release
	}

	return status;
}


NTSTATUS SEND_TCP_DATA(PVOID input_BUFFER, ULONG32 input_BUFFER_SIZE, SOCKET_INFORMATION receive_types) { // 송신할 때

	NTSTATUS status = STATUS_SUCCESS;

	K_object_init_check_also_lock_ifyouwant(&TCP_session_MUTEX, FALSE); // 초기화는 하지만 점유시도는 안함

	if (receive_types != SERVER_DATA_PROCESS) {
		K_object_init_check_also_lock_ifyouwant(&TCP_session_MUTEX, TRUE); // Lock
	}


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_TCP_DATA 크기 -> %d\n", input_BUFFER_SIZE);
	if (Send_or_Receive_TCP_Server(&input_BUFFER_SIZE, sizeof(input_BUFFER_SIZE), TCP_DATA_SEND, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[1/2] 길이 전달
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_TCP_DATA 1 - fail");
		status = STATUS_UNSUCCESSFUL;
		
	}
	else {
		if (Send_or_Receive_TCP_Server(input_BUFFER, input_BUFFER_SIZE, TCP_DATA_SEND, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[2/2] 데이터 전달
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_TCP_DATA 2 - fail");
			status = STATUS_UNSUCCESSFUL;
			
		}
		else {
			status = STATUS_SUCCESS;
		}

	}
	Delays(-1);
	//if (receive_types != SERVER_DATA_PROCESS) {
	K_object_lock_Release(&TCP_session_MUTEX); // Release (무조건 점유해제)
	//}


	return status;
}

VOID RECEIVE_or_SEND_TCP_DATA__with_alloc_Free(PVOID* input_BUFFER_for_freepool) {
	ExFreePoolWithTag(*input_BUFFER_for_freepool, 'RcDt');
	return;
}


