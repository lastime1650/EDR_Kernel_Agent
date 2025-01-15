#ifndef MiniFilter_Handler_H
#define MiniFilter_Handler_H

#include <fltKernel.h>

#include "KEVENT_or_KMUTEX_init.h"

extern K_EVENT_or_MUTEX_struct MiniFilter_mutex;

// �̴� ���� �ڵ鷯���� �����ϴ� ����ü
typedef struct Share_filter_Obj {

    /*
        IRP_MJ_WRITE
    */
    BOOLEAN IsWriteMode;    // ������ ���� ���� ����
    BOOLEAN IsReadMode;     // ���� �б� ����

    KSPIN_LOCK spinlock;
} Share_filter_Obj, * PShare_filter_Obj;

extern PShare_filter_Obj share_filter_object;

typedef enum File_Behavior {
    Create,
    Read,
    Write,
    Rename,
    Delete
}File_Behavior, *PFile_Behavior;


FLT_PREOP_CALLBACK_STATUS
Pre_filter_Handler(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
);


#endif