#include "Get_Time.h"

// --- Buffer Size ---
// YYYY-MM-DDTHH:MM:SS.fffffffffZ (ISO 8601 with nanoseconds)
// 4+1+2+1+2+1+2+1+2+1+2+1+9+1 = 30 characters + 1 null terminator = 31 bytes
#define output_time_buffer_size 25

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

    // 4. ANSI 형식으로 변환 (YYYY-MM-DDTHH:MM:SS.mmmZ) T 와 Z 추가 (ISO 8601 구조)
    RtlStringCbPrintfA(
        OUTPUT_TIME_buffer,
        output_time_buffer_size,
        "%04hu-%02hu-%02huT%02hu:%02hu:%02hu.%03huZ",
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