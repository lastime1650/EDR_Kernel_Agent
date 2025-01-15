#include "Process_Creation_Event.h"

// �δ�
NTSTATUS Process_Creation_Event_Loader() {
	// Process routine Ex �� '������ ����'
	
	return PsSetCreateProcessNotifyRoutineEx(PcreateProcessNotifyRoutineEx,FALSE);
}

// ��δ�
NTSTATUS Process_Creation_Event_Unloader() {
	return PsSetCreateProcessNotifyRoutineEx(PcreateProcessNotifyRoutineEx, TRUE);
}




#include "is_system_pid.h" // �ý��� ���μ��� ����

#include "DynamicData_Linked_list.h" // �����͸� ���Ḯ��Ʈ��
#include "Analysis_enum.h" // ���
#include "DynamicData_2_lengthBuffer.h" // ���Ḯ��Ʈ to ����
#include "Get_Time.h" // ��ýð�

BOOLEAN PROCESS_CREATION_EVENT__data_to_node(
	PEPROCESS input_Process, HANDLE input_ProcessId, PPS_CREATE_NOTIFY_INFO input_CreateInfo,
	PDynamicData* output_startnode
);
// �ڵ鷯
VOID PcreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo) {

	if (CreateInfo) {
		if (Is_it_System_Process(ProcessId))
			return;

		// CMD ����
		Analysis_Command cmd = PsSetCreateProcessNotifyRoutine_Creation_Detail;
		// PID ����
		HANDLE pid = ProcessId;

		// ���� ������ ����
		PDynamicData start_node = NULL;
		if (!PROCESS_CREATION_EVENT__data_to_node(
			Process,
			ProcessId,
			CreateInfo,

			& start_node
		))
			return;

		/*
			���޿� ����������
		*/
		Pmover move = ExAllocatePoolWithTag(NonPagedPool, sizeof(mover), mover_tag);
		if (!move)
			return;

		move->cmd = cmd;
		move->PID = pid;
		move->start_node = start_node;
		move->timestamp = Get_Time();

		// �񵿱� '���̱��'���� ����
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


	/* ������ �� */
	
	// 1. �θ� PID
	*output_startnode = CreateDynamicData((PUCHAR)&input_CreateInfo->ParentProcessId, sizeof(input_CreateInfo->ParentProcessId));
	PDynamicData current = *output_startnode;

	// 2. ������ ID
	current = AppendDynamicData(current, (PUCHAR)&input_CreateInfo->CreatingThreadId.UniqueThread, sizeof(input_CreateInfo->CreatingThreadId.UniqueThread));

	// 3. Ŀ�ǵ����
	ANSI_STRING tmp_str = { 0, };
	UNICODE_to_ANSI(&tmp_str, (PUNICODE_STRING)input_CreateInfo->CommandLine);
	current = AppendDynamicData(current, (PUCHAR)tmp_str.Buffer, tmp_str.MaximumLength-1);
	UNICODE_to_ANSI_release(&tmp_str);

	// 4. EXE �̸�
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