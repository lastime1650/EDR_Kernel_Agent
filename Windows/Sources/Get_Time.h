#ifndef GET_TIME
#define GET_TIME

#include <ntifs.h>
#include <wdm.h>
#include <ntstrsafe.h> // for RtlStringCbPrintfW


// �Լ������� �����Ҵ�� ���ڿ��� �ƿ�ǲ
PCHAR Get_Time();
ULONG32 Time_Length();

// �� �����ؾ���
VOID Release_Got_Time(
	_In_ CHAR* INPUT_buffer       
);

#endif