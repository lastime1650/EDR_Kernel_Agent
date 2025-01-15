#include "Process_Creation_Event.h"

// 로더
NTSTATUS Process_Creation_Event_Loader() {
	// Process routine Ex 로 '생성을 감지'
	
	return PsSetCreateProcessNotifyRoutineEx(PcreateProcessNotifyRoutineEx,FALSE);
}

// 언로더
NTSTATUS Process_Creation_Event_Unloader() {
	return PsSetCreateProcessNotifyRoutineEx(PcreateProcessNotifyRoutineEx, TRUE);
}




#include "is_system_pid.h" // 시스템 프로세스 제외

#include "DynamicData_Linked_list.h" // 데이터를 연결리스트로
#include "Analysis_enum.h" // 명령
#include "DynamicData_2_lengthBuffer.h" // 연결리스트 to 버퍼
#include "Get_Time.h" // 당시시간

BOOLEAN PROCESS_CREATION_EVENT__data_to_node(
	PEPROCESS input_Process, HANDLE input_ProcessId, PPS_CREATE_NOTIFY_INFO input_CreateInfo,
	PDynamicData* output_startnode
);
// 핸들러
VOID PcreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo) {

	if (CreateInfo) {
		if (Is_it_System_Process(ProcessId))
			return;

		// CMD 추출
		Analysis_Command cmd = PsSetCreateProcessNotifyRoutine_Creation_Detail;
		// PID 추출
		HANDLE pid = ProcessId;

		// 동적 데이터 추출
		PDynamicData start_node = NULL;
		if (!PROCESS_CREATION_EVENT__data_to_node(
			Process,
			ProcessId,
			CreateInfo,

			& start_node
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

#include "converter_string.h"
BOOLEAN PROCESS_CREATION_EVENT__data_to_node(
	PEPROCESS input_Process, HANDLE input_ProcessId, PPS_CREATE_NOTIFY_INFO input_CreateInfo,
	PDynamicData* output_startnode
) {
	UNREFERENCED_PARAMETER(input_Process);
	UNREFERENCED_PARAMETER(input_ProcessId);
	BOOLEAN return_bool = TRUE;
	if (!output_startnode) {
		return_bool = FALSE;
		return return_bool;
	}


	/* 데이터 부 */
	
	// 1. 부모 PID
	*output_startnode = CreateDynamicData((PUCHAR)&input_CreateInfo->ParentProcessId, sizeof(input_CreateInfo->ParentProcessId));
	PDynamicData current = *output_startnode;

	// 2. 스레드 ID
	current = AppendDynamicData(current, (PUCHAR)&input_CreateInfo->CreatingThreadId.UniqueThread, sizeof(input_CreateInfo->CreatingThreadId.UniqueThread));

	// 3. 커맨드라인
	ANSI_STRING tmp_str = { 0, };
	UNICODE_to_ANSI(&tmp_str, (PUNICODE_STRING)input_CreateInfo->CommandLine);
	current = AppendDynamicData(current, (PUCHAR)tmp_str.Buffer, tmp_str.MaximumLength-1);
	UNICODE_to_ANSI_release(&tmp_str);

	// 4. EXE 이름
	if (input_CreateInfo->IsSubsystemProcess == FALSE) {
		ANSI_STRING tmp_str2 = { 0, };
		UNICODE_to_ANSI(&tmp_str2, (PUNICODE_STRING)input_CreateInfo->ImageFileName);
		current = AppendDynamicData(current, (PUCHAR)tmp_str2.Buffer, tmp_str2.MaximumLength - 1);
		UNICODE_to_ANSI_release(&tmp_str2);
	}
	else {
		current = AppendDynamicData(current, (PUCHAR)"IsSubsystemProcess", sizeof("IsSubsystemProcess") - 1);
	}
	
	

	return return_bool;


}