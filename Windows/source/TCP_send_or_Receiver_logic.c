#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "TCP_send_or_Receiver.h"
#include "util_Delay.h"


K_EVENT_or_MUTEX_struct TCP_session_MUTEX = { NULL, K_MUTEX, FALSE };

/*
	[�۽�]
		1. ���� -> �۽� * 2 -> �������� -> ��ȯ
		2. �۽�	* 2 -> ��ȯ 

	[����]
		1. ���� -> ���� * 2 -> �������� -> ��ȯ
		2. ���� -> ���� * 2 -> ��ȯ

*/


NTSTATUS RECEIVE_TCP_DATA__with_alloc(PVOID* output_BUFFER, ULONG32* output_BUFFER_SIZE, SOCKET_INFORMATION receive_types) { // ������ ��

	K_object_init_check_also_lock_ifyouwant(&TCP_session_MUTEX, FALSE); // �ʱ�ȭ�� ������ �����õ��� ����

	/*
		�� 2�� �������� Receive��
		[1/2]
			4����Ʈ�� SERVER�κ��� �о ������ ��ȯ�Ͽ� ����
		[2/2]
			([1/2]) ���� �͸�ŭ�� ũ��� �����Ҵ���
	*/
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc -> TCP ���ؽ� �����õ� \n");
	K_object_init_check_also_lock_ifyouwant(&TCP_session_MUTEX, TRUE); // Lock
	NTSTATUS status = STATUS_SUCCESS;

	*output_BUFFER = NULL;
	ULONG32 allocate_size = 0;
	if (Send_or_Receive_TCP_Server(&allocate_size, sizeof(allocate_size), TCP_DATA_RECEIVE, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[1/2]
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 2 (����)  \n");
		status = STATUS_UNSUCCESSFUL;
	}
	else {
		if (allocate_size == 0) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 2-1 (����) allocate_size���̰� 0 ��  \n");
			status = STATUS_UNSUCCESSFUL;
		}

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 3 (1�� ����) �����κ��� ������ ���� -> %lu \n", allocate_size);
		*output_BUFFER_SIZE = allocate_size; // ����ũ��
		*output_BUFFER = (PVOID)ExAllocatePoolWithTag(PagedPool, *output_BUFFER_SIZE, 'RcDt'); // �����Ҵ�
		if (*output_BUFFER == NULL || allocate_size == 0) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 4 \n");
			status = STATUS_INSUFFICIENT_RESOURCES;
		}
		else {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 5 ( 2���� ����ϰ� �ֽ��ϴ�.. )  \n");
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


NTSTATUS SEND_TCP_DATA(PVOID input_BUFFER, ULONG32 input_BUFFER_SIZE, SOCKET_INFORMATION receive_types) { // �۽��� ��

	NTSTATUS status = STATUS_SUCCESS;

	K_object_init_check_also_lock_ifyouwant(&TCP_session_MUTEX, FALSE); // �ʱ�ȭ�� ������ �����õ��� ����

	if (receive_types != SERVER_DATA_PROCESS) {
		K_object_init_check_also_lock_ifyouwant(&TCP_session_MUTEX, TRUE); // Lock
	}


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_TCP_DATA ũ�� -> %d\n", input_BUFFER_SIZE);
	if (Send_or_Receive_TCP_Server(&input_BUFFER_SIZE, sizeof(input_BUFFER_SIZE), TCP_DATA_SEND, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[1/2] ���� ����
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_TCP_DATA 1 - fail");
		status = STATUS_UNSUCCESSFUL;
		
	}
	else {
		if (Send_or_Receive_TCP_Server(input_BUFFER, input_BUFFER_SIZE, TCP_DATA_SEND, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[2/2] ������ ����
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_TCP_DATA 2 - fail");
			status = STATUS_UNSUCCESSFUL;
			
		}
		else {
			status = STATUS_SUCCESS;
		}

	}
	Delays(-1);
	//if (receive_types != SERVER_DATA_PROCESS) {
	K_object_lock_Release(&TCP_session_MUTEX); // Release (������ ��������)
	//}


	return status;
}

VOID RECEIVE_or_SEND_TCP_DATA__with_alloc_Free(PVOID* input_BUFFER_for_freepool) {
	ExFreePoolWithTag(*input_BUFFER_for_freepool, 'RcDt');
	return;
}


