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