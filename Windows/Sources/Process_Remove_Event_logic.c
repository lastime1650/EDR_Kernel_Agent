#include "Process_Remove_Event.h"

// 로더
NTSTATUS Process_Remove_Event_Loader() {
	// Process routine Ex 로 '제거를 감지'

	return PsSetCreateProcessNotifyRoutine(PcreateProcessNotifyRoutine, FALSE);
}

// 언로더
NTSTATUS Process_Remove_Event_Unloader() {
	return PsSetCreateProcessNotifyRoutine(PcreateProcessNotifyRoutine, TRUE);
}

// 핸들러
#include "is_system_pid.h" // 시스템 프로세스 제외

#include "DynamicData_Linked_list.h" // 데이터를 연결리스트로
#include "Analysis_enum.h" // 명령
#include "DynamicData_2_lengthBuffer.h" // 연결리스트 to 버퍼
#include "Get_Time.h" // 당시시간
BOOLEAN PROCESS_Remove_EVENT__data_to_node(
	HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create,
	PDynamicData* output_startnode
);
VOID PcreateProcessNotifyRoutine(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create) {


	if (!Create) {
		
		if (Is_it_System_Process(ProcessId))
			return;

		// CMD 추출
		Analysis_Command cmd = PsSetCreateProcessNotifyRoutine_Remove;;
		// PID 추출
		HANDLE pid = ProcessId;


		// 동적 데이터 추출
		PDynamicData start_node = NULL;
		if (!PROCESS_Remove_EVENT__data_to_node(
			ParentId,
			ProcessId,
			Create,

			&start_node
		))
			return;



		/*
			전달용 동적데이터
		*/
		Pmover move = ExAllocatePoolWithTag(NonPagedPool, sizeof(mover), mover_tag);
		if (!move)
			return;

		move->cmd = cmd;
		move->PID = pid;
		move->start_node = start_node;
		move->timestamp = Get_Time();

		// 비동기 '길이기반'버퍼 생성
		HANDLE threadid = 0;
		PsCreateSystemThread(
			&threadid,
			THREAD_ALL_ACCESS,
			NULL,
			NULL,
			NULL,
			Dyn_2_lenBuff,
			move
		);
		ZwClose(threadid);
	}
	return;
}


BOOLEAN PROCESS_Remove_EVENT__data_to_node(
	HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create,
	PDynamicData* output_startnode
) {
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(Create);
	BOOLEAN return_bool = TRUE;
	if (!output_startnode) {
		return_bool = FALSE;
		return return_bool;
	}

	/* 데이터 부 */

	// 1. 부모 PID
	*output_startnode = CreateDynamicData((PUCHAR)&ParentId, sizeof(ParentId));

	return return_bool;
}