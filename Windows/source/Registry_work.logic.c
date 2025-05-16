#include "Registry_work.h"

#include "DynamicData_2_lengthBuffer.h"
VOID Registry_work_job(PWORK_ITEM work) {
	if (!work)
		return;

	Pmover move = (Pmover)work->context.context;

	// 비동기 '길이기반'버퍼 생성
	HANDLE threadid = 0;
	if (PsCreateSystemThread(
		&threadid,
		THREAD_ALL_ACCESS,
		NULL,
		NULL,
		NULL,
		Dyn_2_lenBuff,
		move
	) != STATUS_SUCCESS) {
		RemoveALLDynamicData(move->start_node);
		ExFreePoolWithTag(move, mover_tag);
	}

	ExFreePoolWithTag(work, WORK_ITEM_TAG);
}