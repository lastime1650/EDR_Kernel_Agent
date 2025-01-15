#include "Get_Time.h"

#define output_time_buffer_size 24

PCHAR Get_Time()
{

	PCHAR OUTPUT_TIME_buffer = (PCHAR)ExAllocatePool(NonPagedPool, output_time_buffer_size);
	if (OUTPUT_TIME_buffer == NULL) return NULL;

    LARGE_INTEGER systemTime, localTime;
    TIME_FIELDS timeFields;

    // 1. 현재 시스템 시간을 얻기
    KeQuerySystemTimePrecise(&systemTime);

    // 2. 시스템 시간을 로컬 시간으로 변환
    ExSystemTimeToLocalTime(&systemTime, &localTime);

    // 3. 로컬 시간을 TIME_FIELDS 구조체로 변환
    RtlTimeToTimeFields(&localTime, &timeFields);

    // 4. ANSI 형식으로 변환 (YYYY-MM-DD HH:MM:SS.mmm)
    RtlStringCbPrintfA(
        OUTPUT_TIME_buffer,
        output_time_buffer_size,// 길이 input ( null 포함 ) 
        "%04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu",
        timeFields.Year,
        timeFields.Month,
        timeFields.Day,
        timeFields.Hour,
        timeFields.Minute,
        timeFields.Second,
        timeFields.Milliseconds
    );
    

    return OUTPUT_TIME_buffer;
}

ULONG32 Time_Length() {
	return output_time_buffer_size;
}


VOID Release_Got_Time(
    _In_ CHAR* INPUT_buffer      // 시간 정보를 담을 버퍼
) {
	if (!INPUT_buffer) 
        return;
	ExFreePool(INPUT_buffer);
}