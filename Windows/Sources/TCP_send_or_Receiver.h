#ifndef TCP_send_or_Receiver_H
#define TCP_send_or_Receiver_H

#include <ntifs.h>
#include <ntddk.h>

#include "Socket.h"
#include "KEVENT_or_KMUTEX_init.h"

// 전역변수
extern K_EVENT_or_MUTEX_struct TCP_session_MUTEX;

/*
	( SOCKET_INFORMATION receive_types ) <- Build_Socket.h

*/

// 수신
NTSTATUS RECEIVE_TCP_DATA__with_alloc(PVOID* output_BUFFER, ULONG32* output_BUFFER_SIZE, SOCKET_INFORMATION receive_types);


// 송신
NTSTATUS SEND_TCP_DATA(PVOID input_BUFFER, ULONG32 input_BUFFER_SIZE, SOCKET_INFORMATION receive_types);


// 수신받고, 수신받은 데이터를 동적해제하는 함수
VOID RECEIVE_or_SEND_TCP_DATA__with_alloc_Free(PVOID* input_BUFFER_for_freepool);
#endif




