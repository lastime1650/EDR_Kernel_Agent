#include "Image_Load_Event.h"
// 로더
NTSTATUS Image_Load_Event_Loader() {
	return PsSetLoadImageNotifyRoutine(PLoadImageNotifyRoutine);
}

// 언로더
NTSTATUS Image_Load_Event_Unloader() {
	return PsRemoveLoadImageNotifyRoutine(PLoadImageNotifyRoutine);
}

// 핸들러
#include "is_system_pid.h" // 시스템 프로세스 제외

#include "DynamicData_Linked_list.h" // 데이터를 연결리스트로
#include "Analysis_enum.h" // 명령
#include "DynamicData_2_lengthBuffer.h" // 연결리스트 to 버퍼
#include "Get_Time.h" // 당시시간

BOOLEAN IMAGE_Load_EVENT__data_to_node(
	PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo,
	PDynamicData* output_startnode
);
VOID PLoadImageNotifyRoutine(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo) {

	if (Is_it_System_Process(ProcessId))
		return;

	// CMD 추출
	Analysis_Command cmd = PsSetLoadImageNotifyRoutine_Load;
	// PID 추출
	HANDLE pid = ProcessId;

	// 동적 데이터 추출
	PDynamicData start_node = NULL;
	if (!IMAGE_Load_EVENT__data_to_node(
		FullImageName,
		ProcessId,
		ImageInfo,

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

	/* 데이터 부 */
	
	// 1. Image 절대경로 ( pwch ) 
	if (FullImageName != NULL ) {
		*output_startnode = CreateDynamicData((PUCHAR)FullImageName->Buffer, FullImageName->MaximumLength);
	}
	else {
		return FALSE;
	}
	PDynamicData current = *output_startnode;

	// 2. Image 파일 사이즈 
	current = AppendDynamicData(current, (PUCHAR)&ImageInfo->ImageSize, sizeof(ImageInfo->ImageSize));


	return return_bool;


}