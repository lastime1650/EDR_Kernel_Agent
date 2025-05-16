/*
    누수 발생 여부: 양호
*/
#include "WorkItem_job.h"

#include "DynamicData_2_lengthBuffer.h"
#include "DynamicData_Linked_list.h"

VOID WORK_job(PWORK_ITEM input_parm) {
	if (!input_parm)
		return;
    Pmover move = (Pmover)input_parm->context.context;
    if (!move) {
        goto EXIT;
    }

    HANDLE handle;
    if (PsCreateSystemThread(
        &handle,
        THREAD_ALL_ACCESS,
        NULL,
        NULL,
        NULL,
        Dyn_2_lenBuff,
        move
    ) != STATUS_SUCCESS) {
        ExFreePoolWithTag(move, mover_tag);
        goto EXIT_1;

    }
    ZwClose(handle);

    goto EXIT_1;


EXIT_1:
    {
        goto EXIT;
    }
EXIT:
{
        ExFreePoolWithTag(input_parm, WORK_ITEM_TAG);
        return;
    }
}
