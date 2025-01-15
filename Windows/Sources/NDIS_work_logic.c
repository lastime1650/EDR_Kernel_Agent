#include "NDIS_work.h"
/* 사용안함 */
VOID NDIS_WORK_job(PWORK_ITEM input_parm) {
    if (!input_parm)
        return;
    Pmover move = (Pmover)input_parm->context.context;

    HANDLE threadid = 0;
    if (PsCreateSystemThread(
        &threadid,
        THREAD_ALL_ACCESS,
        NULL,
        NULL,
        NULL,
        input_parm->context.startroutine,
        move
    ) != STATUS_SUCCESS) {
        RemoveALLDynamicData(move->start_node);
        ExFreePoolWithTag(move, mover_tag);
    }

    ExFreePoolWithTag(input_parm, WORK_ITEM_TAG);
    return;
}
