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
#include "Get_PPID_with_ImageName.h"
BOOLEAN PROCESS_CREATION_EVENT__data_to_node(
	PEPROCESS input_Process, HANDLE input_ProcessId, PPS_CREATE_NOTIFY_INFO input_CreateInfo,
	PDynamicData* output_startnode
);
// 핸들러
VOID PcreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo) {

	if (CreateInfo) {

		PCHAR Timestamp = Get_Time();

		if (Is_it_System_Process(ProcessId)) {
			Release_Got_Time(Timestamp);
			return;
		}
			

		

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

			&start_node
		)) {
			Release_Got_Time(Timestamp);
			return;
		}
			

		/*
			전달용 동적데이터
		*/
		Pmover move = ExAllocatePoolWithTag(NonPagedPool, sizeof(mover), mover_tag);
		if (!move)
			return;

		move->cmd = cmd;
		move->PID = pid;
		move->start_node = start_node;
		move->timestamp = Timestamp;

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

#include "File_io.h"
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

	// 4. EXE 이미지 이름
	ULONG32 exe_size = 0;
	CHAR SHA256[SHA256_String_Byte_Length] = { 0, };
	FILE_to_INFO(
		(PUNICODE_STRING)input_CreateInfo->ImageFileName,
		NULL,
		&exe_size,
		NULL,
		SHA256
	);

	// 자기 이미지 이름
	UNICODE_to_ANSI(&tmp_str, (PUNICODE_STRING)input_CreateInfo->ImageFileName);
	current = AppendDynamicData(current, (PUCHAR)tmp_str.Buffer, tmp_str.MaximumLength-1 );
	UNICODE_to_ANSI_release(&tmp_str);

	// sha256
	current = AppendDynamicData(current, (PUCHAR)SHA256, (SHA256_String_Byte_Length-1));

	// 길이
	current = AppendDynamicData(current, (PUCHAR)&exe_size, sizeof(exe_size));



	// 5. 부모 프로세스의 "이미지" 이름
	// [+] -> 사용자가 직접 실행한 프로세스인지 검증하는 로직이 추가됨.
		// 프로세스 시작시,,,, 부모프로세스를 기반으로하여 explorer.exe일 때 부모로 확인( 확인작업은 CoreServer에서 진행한다)

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "0 [프로세스생성] - 자신 정보 PID: %llu // ImageName: %wZ\n", input_ProcessId, input_CreateInfo->ImageFileName);

	PUNICODE_STRING Parent_ImageName = NULL;
	HANDLE PPID = 0;
	Get_PPID_with_ImageName(
		input_ProcessId,
		&Parent_ImageName,
		&PPID
	);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "1 [프로세스생성] - 부모 정보 PPID: %llu // ImageName: %wZ\n", PPID, Parent_ImageName);

	FILE_to_INFO(
		Parent_ImageName,
		NULL,
		&exe_size,
		NULL,
		SHA256
	);

	// 이미지 이름
	UNICODE_to_ANSI(&tmp_str, (PUNICODE_STRING)Parent_ImageName);
	current = AppendDynamicData(current, (PUCHAR)tmp_str.Buffer, tmp_str.MaximumLength - 1);
	UNICODE_to_ANSI_release(&tmp_str);

	// sha256
	current = AppendDynamicData(current, (PUCHAR)SHA256, (SHA256_String_Byte_Length-1));

	// 길이
	current = AppendDynamicData(current, (PUCHAR)&exe_size, sizeof(exe_size));

	FREE_get_PPID_with_ImageName(Parent_ImageName);
	//
	


	return return_bool;


}