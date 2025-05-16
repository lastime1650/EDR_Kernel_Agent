#ifndef MnFilter_H
#define MnFilter_H

#include <fltKernel.h>

#include "MiniFilter_Handler.h" // 핸들러


// 등록 함수
NTSTATUS Register_MiniFilter(PDRIVER_OBJECT Input_Driver_obj);

// 해제
NTSTATUS Unload_Minifilter();

// 콜백 모아둔 곳
extern const FLT_OPERATION_REGISTRATION Callback_s[];
 
extern PFLT_FILTER gFilterHandle;



#endif 