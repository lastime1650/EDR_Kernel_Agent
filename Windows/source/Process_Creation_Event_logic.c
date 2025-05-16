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
#include "Get_PPID_with_ImageName.h"
BOOLEAN PROCESS_CREATION_EVENT__data_to_node(
	PEPROCESS input_Process, HANDLE input_ProcessId, PPS_CREATE_NOTIFY_INFO input_CreateInfo,
	PDynamicData* output_startnode
);
// �ڵ鷯
VOID PcreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo) {

	if (CreateInfo) {

		PCHAR Timestamp = Get_Time();

		if (Is_it_System_Process(ProcessId)) {
			Release_Got_Time(Timestamp);
			return;
		}
			

		

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

			&start_node
		)) {
			Release_Got_Time(Timestamp);
			return;
		}
			

		/*
			���޿� ����������
		*/
		Pmover move = ExAllocatePoolWithTag(NonPagedPool, sizeof(mover), mover_tag);
		if (!move)
			return;

		move->cmd = cmd;
		move->PID = pid;
		move->start_node = start_node;
		move->timestamp = Timestamp;

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

	// 4. EXE �̹��� �̸�
	ULONG32 exe_size = 0;
	CHAR SHA256[SHA256_String_Byte_Length] = { 0, };
	FILE_to_INFO(
		(PUNICODE_STRING)input_CreateInfo->ImageFileName,
		NULL,
		&exe_size,
		NULL,
		SHA256
	);

	// �ڱ� �̹��� �̸�
	UNICODE_to_ANSI(&tmp_str, (PUNICODE_STRING)input_CreateInfo->ImageFileName);
	current = AppendDynamicData(current, (PUCHAR)tmp_str.Buffer, tmp_str.MaximumLength-1 );
	UNICODE_to_ANSI_release(&tmp_str);

	// sha256
	current = AppendDynamicData(current, (PUCHAR)SHA256, (SHA256_String_Byte_Length-1));

	// ����
	current = AppendDynamicData(current, (PUCHAR)&exe_size, sizeof(exe_size));



	// 5. �θ� ���μ����� "�̹���" �̸�
	// [+] -> ����ڰ� ���� ������ ���μ������� �����ϴ� ������ �߰���.
		// ���μ��� ���۽�,,,, �θ����μ����� ��������Ͽ� explorer.exe�� �� �θ�� Ȯ��( Ȯ���۾��� CoreServer���� �����Ѵ�)

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "0 [���μ�������] - �ڽ� ���� PID: %llu // ImageName: %wZ\n", input_ProcessId, input_CreateInfo->ImageFileName);

	PUNICODE_STRING Parent_ImageName = NULL;
	HANDLE PPID = 0;
	Get_PPID_with_ImageName(
		input_ProcessId,
		&Parent_ImageName,
		&PPID
	);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "1 [���μ�������] - �θ� ���� PPID: %llu // ImageName: %wZ\n", PPID, Parent_ImageName);

	FILE_to_INFO(
		Parent_ImageName,
		NULL,
		&exe_size,
		NULL,
		SHA256
	);

	// �̹��� �̸�
	UNICODE_to_ANSI(&tmp_str, (PUNICODE_STRING)Parent_ImageName);
	current = AppendDynamicData(current, (PUCHAR)tmp_str.Buffer, tmp_str.MaximumLength - 1);
	UNICODE_to_ANSI_release(&tmp_str);

	// sha256
	current = AppendDynamicData(current, (PUCHAR)SHA256, (SHA256_String_Byte_Length-1));

	// ����
	current = AppendDynamicData(current, (PUCHAR)&exe_size, sizeof(exe_size));

	FREE_get_PPID_with_ImageName(Parent_ImageName);
	//
	


	return return_bool;


}