#ifndef util_Delay_H
#define util_Delay_H

#include <ntddk.h>

VOID Delays(INT per_sec);
VOID Delay_with_milli(ULONG milliseconds);// 100 넣으면 0.1초대기
#endif 