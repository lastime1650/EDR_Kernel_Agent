#include "util_Delay.h"

VOID Delays(INT per_sec) {
	/*
		1초한다면 per_sec 값이 -1 이어야함
	*/
	LARGE_INTEGER interval;

	interval.QuadPart = 1000 * 1000 * 10;
	interval.QuadPart *= per_sec;

	KeDelayExecutionThread(KernelMode, FALSE, &interval); //대기시작!

	return;
}

VOID Delay_with_milli(ULONG milliseconds) { // ULONG은 unsigned long
	LARGE_INTEGER interval;

	// 밀리초를 100나노초 단위로 변환
	interval.QuadPart = (LONGLONG)(-1 * milliseconds * 10000);

	KeDelayExecutionThread(KernelMode, FALSE, &interval);
}