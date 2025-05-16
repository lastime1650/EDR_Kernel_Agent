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

#include "API_start_hook.h" // API ��ŷ ����

BOOLEAN IMAGE_Load_EVENT__data_to_node(
	PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo,
	PDynamicData* output_startnode
);
VOID PLoadImageNotifyRoutine(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo) {

	PCHAR Timestamp = Get_Time();

	if (Is_it_System_Process(ProcessId)) {
		Release_Got_Time(Timestamp);
		return;
	}
		

	// ntdll.dll �ε��� ��, ��ŷ �õ�
	//Lets_Hook(ProcessId, FullImageName, ImageInfo->ImageBase);

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
	)) {
		Release_Got_Time(Timestamp);
		return;
	}
		

	/*
		���޿� ����������
	*/
	Pmover move = ExAllocatePoolWithTag(NonPagedPool, sizeof(mover), mover_tag);
	if (!move) {
		Release_Got_Time(Timestamp);
		return;
	}
		

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

#include "converter_string.h"
#include "File_io.h"
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
		ANSI_STRING tmp_str2 = { 0, };
		UNICODE_to_ANSI(&tmp_str2, (PUNICODE_STRING)FullImageName);
		*output_startnode = CreateDynamicData((PUCHAR)tmp_str2.Buffer, tmp_str2.MaximumLength-1 );
		UNICODE_to_ANSI_release(&tmp_str2);
		//*output_startnode = CreateDynamicData((PUCHAR)&FullImageName->Buffer, FullImageName->MaximumLength+sizeof(WCHAR));
	}
	else {
		return FALSE;
	}
	PDynamicData current = *output_startnode;

	// 2. Image ���� ������ 
	current = AppendDynamicData(current, (PUCHAR)&ImageInfo->ImageSize, sizeof(ImageInfo->ImageSize));

	// 3. Image ���� SHA256 ��������
	CHAR SHA256[SHA256_String_Byte_Length] = { 0, };
	FILE_to_INFO(
		(PUNICODE_STRING)FullImageName,
		NULL,
		NULL,
		NULL,
		SHA256
	);

	// sha256
	current = AppendDynamicData(current, (PUCHAR)SHA256, (SHA256_String_Byte_Length - 1));


	return return_bool;


}