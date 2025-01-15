#include "Process_Remove_Event.h"

// �δ�
NTSTATUS Process_Remove_Event_Loader() {
	// Process routine Ex �� '���Ÿ� ����'

	return PsSetCreateProcessNotifyRoutine(PcreateProcessNotifyRoutine, FALSE);
}

// ��δ�
NTSTATUS Process_Remove_Event_Unloader() {
	return PsSetCreateProcessNotifyRoutine(PcreateProcessNotifyRoutine, TRUE);
}

// �ڵ鷯
#include "is_system_pid.h" // �ý��� ���μ��� ����

#include "DynamicData_Linked_list.h" // �����͸� ���Ḯ��Ʈ��
#include "Analysis_enum.h" // ���
#include "DynamicData_2_lengthBuffer.h" // ���Ḯ��Ʈ to ����
#include "Get_Time.h" // ��ýð�
BOOLEAN PROCESS_Remove_EVENT__data_to_node(
	HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create,
	PDynamicData* output_startnode
);
VOID PcreateProcessNotifyRoutine(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create) {


	if (!Create) {
		
		if (Is_it_System_Process(ProcessId))
			return;

		// CMD ����
		Analysis_Command cmd = PsSetCreateProcessNotifyRoutine_Remove;;
		// PID ����
		HANDLE pid = ProcessId;


		// ���� ������ ����
		PDynamicData start_node = NULL;
		if (!PROCESS_Remove_EVENT__data_to_node(
			ParentId,
			ProcessId,
			Create,

			&start_node
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

	/* ������ �� */

	// 1. �θ� PID
	*output_startnode = CreateDynamicData((PUCHAR)&ParentId, sizeof(ParentId));

	return return_bool;
}