#ifndef MnFilter_H
#define MnFilter_H

#include <fltKernel.h>

#include "MiniFilter_Handler.h" // �ڵ鷯


// ��� �Լ�
NTSTATUS Register_MiniFilter(PDRIVER_OBJECT Input_Driver_obj);

// ����
NTSTATUS Unload_Minifilter();

// �ݹ� ��Ƶ� ��
extern const FLT_OPERATION_REGISTRATION Callback_s[];
 
extern PFLT_FILTER gFilterHandle;



#endif 