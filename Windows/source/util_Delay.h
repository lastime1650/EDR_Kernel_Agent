#ifndef util_Delay_H
#define util_Delay_H

#include <ntddk.h>

VOID Delays(INT per_sec);
VOID Delay_with_milli(ULONG milliseconds);// 100 ������ 0.1�ʴ��
#endif 