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

VOID Delay_with_milli(ULONG milliseconds) { // ULONG�� unsigned long
	LARGE_INTEGER interval;

	// �и��ʸ� 100������ ������ ��ȯ
	interval.QuadPart = (LONGLONG)(-1 * milliseconds * 10000);

	KeDelayExecutionThread(KernelMode, FALSE, &interval);
}