#ifndef GET_TIME
#define GET_TIME

#include <ntifs.h>
#include <wdm.h>
#include <ntstrsafe.h> // for RtlStringCbPrintfW


// 함수내에서 동적할당된 문자열을 아웃풋
PCHAR Get_Time();
ULONG32 Time_Length();

// 꼭 해제해야함
VOID Release_Got_Time(
	_In_ CHAR* INPUT_buffer       
);

#endif