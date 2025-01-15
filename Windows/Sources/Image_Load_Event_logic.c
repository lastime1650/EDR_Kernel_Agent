#include "Image_Load_Event.h"
// �δ�
NTSTATUS Image_Load_Event_Loader() {
	return PsSetLoadImageNotifyRoutine(PLoadImageNotifyRoutine);
}

// ��δ�
NTSTATUS Image_Load_Event_Unloader() {
	return PsRemoveLoadImageNotifyRoutine(PLoadImageNotifyRoutine);
}

// �ڵ鷯
#include "is_system_pid.h" // �ý��� ���μ��� ����

#include "DynamicData_Linked_list.h" // �����͸� ���Ḯ��Ʈ��
#include "Analysis_enum.h" // ���
#include "DynamicData_2_lengthBuffer.h" // ���Ḯ��Ʈ to ����
#include "Get_Time.h" // ��ýð�

BOOLEAN IMAGE_Load_EVENT__data_to_node(
	PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo,
	PDynamicData* output_startnode
);
VOID PLoadImageNotifyRoutine(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo) {

	if (Is_it_System_Process(ProcessId))
		return;

	// CMD ����
	Analysis_Command cmd = PsSetLoadImageNotifyRoutine_Load;
	// PID ����
	HANDLE pid = ProcessId;

	// ���� ������ ����
	PDynamicData start_node = NULL;
	if (!IMAGE_Load_EVENT__data_to_node(
		FullImageName,
		ProcessId,
		ImageInfo,

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

#include "converter_string.h"
BOOLEAN IMAGE_Load_EVENT__data_to_node(
	PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo,
	PDynamicData* output_startnode
) {
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(ImageInfo);
	BOOLEAN return_bool = TRUE;
	if (!output_startnode) {
		return_bool = FALSE;
		return return_bool;
	}

	/* ������ �� */
	
	// 1. Image ������ ( pwch ) 
	if (FullImageName != NULL ) {
		*output_startnode = CreateDynamicData((PUCHAR)FullImageName->Buffer, FullImageName->MaximumLength);
	}
	else {
		return FALSE;
	}
	PDynamicData current = *output_startnode;

	// 2. Image ���� ������ 
	current = AppendDynamicData(current, (PUCHAR)&ImageInfo->ImageSize, sizeof(ImageInfo->ImageSize));


	return return_bool;


}