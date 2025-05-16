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

    // 1. ���� �ý��� �ð��� ���
    KeQuerySystemTimePrecise(&systemTime);

    // 2. �ý��� �ð��� ���� �ð����� ��ȯ
    ExSystemTimeToLocalTime(&systemTime, &localTime);

    // 3. ���� �ð��� TIME_FIELDS ����ü�� ��ȯ
    RtlTimeToTimeFields(&localTime, &timeFields);

    // 4. ANSI �������� ��ȯ (YYYY-MM-DDTHH:MM:SS.mmmZ) T �� Z �߰� (ISO 8601 ����)
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
    _In_ CHAR* INPUT_buffer      // �ð� ������ ���� ����
) {
	if (!INPUT_buffer) 
        return;
	ExFreePool(INPUT_buffer);
}