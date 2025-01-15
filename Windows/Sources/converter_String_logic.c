#include "converter_string.h"

/*UNICODE_STRING --> ANSI_STRING*/
NTSTATUS UNICODE_to_ANSI(ANSI_STRING* OUTPUT_ansi, UNICODE_STRING* INPUT_unicode) {

	NTSTATUS status = STATUS_SUCCESS;

	status = RtlUnicodeStringToAnsiString(OUTPUT_ansi, (PCUNICODE_STRING)INPUT_unicode, TRUE);
	if (status != STATUS_SUCCESS) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "UNICODE_ 값이 ANSI 로 변환 실패");

	}
	return status;

}


/*ANSI_STRING --> UNICODE_STRING*/
NTSTATUS ANSI_to_UNICODE(UNICODE_STRING* OUTPUT_unicode, ANSI_STRING INPUT_ansi) {

	NTSTATUS status = STATUS_SUCCESS;
	RtlInitEmptyUnicodeString(OUTPUT_unicode, NULL, 0);

	status = RtlAnsiStringToUnicodeString(OUTPUT_unicode, (PCANSI_STRING)&INPUT_ansi, TRUE);
	if (status != STATUS_SUCCESS) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ANSI 값이 UNICODE 로 변환 실패");

	}

	status = RtlValidateUnicodeString(0, OUTPUT_unicode);
	if (!NT_SUCCESS(status)) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ANSI 값이 UNICODE 로 유효성 실패");
	}


	return status;

}
/* PWCH -> ANSI_STRING */
NTSTATUS PWCH_to_ANSI(ANSI_STRING* OUTPUT_ansi, PWCH INPUT_pwch) {
	NTSTATUS status;

	UNICODE_STRING tmp_unicode = { 0, };
	RtlInitUnicodeString(&tmp_unicode, INPUT_pwch);

	ANSI_STRING ansi = { 0, };

	status = UNICODE_to_ANSI(&ansi, &tmp_unicode);

	*OUTPUT_ansi = ansi;

	return status;
}




VOID UNICODE_to_ANSI_release(ANSI_STRING* INPUT_ansi) {
	if (!INPUT_ansi)
		return;
	RtlFreeAnsiString(INPUT_ansi);
}
VOID ANSI_to_UNICODE_release(UNICODE_STRING* INPUT_unicode) {
	if (!INPUT_unicode)
		return;
	RtlFreeUnicodeString(INPUT_unicode);
}
VOID PWCH_to_ANSI_release(ANSI_STRING* INPUT_ansi) {
	UNICODE_to_ANSI_release(INPUT_ansi);
}