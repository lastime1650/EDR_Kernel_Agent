#include "util_Delay.h"

VOID Delays(INT per_sec) {
	/*
		1���Ѵٸ� per_sec ���� -1 �̾����
	*/
	LARGE_INTEGER interval;

	interval.QuadPart = 1000 * 1000 * 10;
	interval.QuadPart *= per_sec;

	KeDelayExecutionThread(KernelMode, FALSE, &interval); //������!

	return;
}