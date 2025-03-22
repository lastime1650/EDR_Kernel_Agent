#include "Start_TCP_session.h"


#include "Socket.h"
#include "util_Delay.h"





#include "Init_session.h"
#include "session_comunicate.h"

VOID Loop_for_TCP_Session(PVOID Context){
	NTSTATUS status;
	UNREFERENCED_PARAMETER(Context);

	// �ٸ� �����忡�� �۾��� �����ϸ� ��ŵ� �����ϵ��� �����
	K_EVENT_or_MUTEX_struct loop_tcp_session_event = { NULL, K_EVENT, FALSE };

	K_object_init_check_also_lock_ifyouwant(&loop_tcp_session_event, FALSE); // �ʱ�ȭ�� ������ �̺�Ʈ �����õ��� ����

	while (1) {

		if (Make_TCP_Connection("192.168.0.1", 10299, &COMMAND_NewSocket) == STATUS_SUCCESS) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "EDR�� �����\n");
			// SMBIOS Type 1 + 2 ���Ͽ� EDR���� ����
			status = INIT_SESSION();
			if (status != STATUS_SUCCESS)
				continue;

			// ���� ��� ����
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

			// ������ ���
			K_object_init_check_also_lock_ifyouwant(&loop_tcp_session_event, TRUE); // �̺�Ʈ ����

			// ������� ����

		}
		else {
			Delays(-1);
			
		}

		TCP_socket_release(&COMMAND_NewSocket);// Make_TCP_Connection ȣ���� ������ �� �ʱ�ȭ�� �����ϵ��� ����.
		continue;
	}

}