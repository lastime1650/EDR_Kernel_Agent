#include "Get_Time.h"

#define output_time_buffer_size 24

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

    // 4. ANSI �������� ��ȯ (YYYY-MM-DD HH:MM:SS.mmm)
    RtlStringCbPrintfA(
        OUTPUT_TIME_buffer,
        output_time_buffer_size,// ���� input ( null ���� ) 
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
    _In_ CHAR* INPUT_buffer      // �ð� ������ ���� ����
) {
	if (!INPUT_buffer) 
        return;
	ExFreePool(INPUT_buffer);
}