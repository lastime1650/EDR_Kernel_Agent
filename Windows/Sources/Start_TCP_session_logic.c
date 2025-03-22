#include "Start_TCP_session.h"


#include "Socket.h"
#include "util_Delay.h"





#include "Init_session.h"
#include "session_comunicate.h"

VOID Loop_for_TCP_Session(PVOID Context){
	NTSTATUS status;
	UNREFERENCED_PARAMETER(Context);

	// 다른 스레드에서 작업이 실패하면 통신도 실패하도록 설계됨
	K_EVENT_or_MUTEX_struct loop_tcp_session_event = { NULL, K_EVENT, FALSE };

	K_object_init_check_also_lock_ifyouwant(&loop_tcp_session_event, FALSE); // 초기화는 하지만 이벤트 점유시도는 안함

	while (1) {

		if (Make_TCP_Connection("192.168.0.1", 10299, &COMMAND_NewSocket) == STATUS_SUCCESS) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "EDR과 연결됨\n");
			// SMBIOS Type 1 + 2 더하여 EDR에게 전송
			status = INIT_SESSION();
			if (status != STATUS_SUCCESS)
				continue;

			// 세션 통신 시작
			HANDLE thread;
			PsCreateSystemThread(
				&thread,
				THREAD_ALL_ACCESS,
				NULL,
				0,
				NULL,
				Session_Communication,
				&loop_tcp_session_event
			);

			// 스레드 잠금
			K_object_init_check_also_lock_ifyouwant(&loop_tcp_session_event, TRUE); // 이벤트 점유

			// 여기부터 종료

		}
		else {
			Delays(-1);
			
		}

		TCP_socket_release(&COMMAND_NewSocket);// Make_TCP_Connection 호출할 때마다 재 초기화가 진행하도록 설계.
		continue;
	}

}