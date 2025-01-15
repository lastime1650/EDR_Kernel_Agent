#include "PROCESS_Request_Real_File.h"

#include "converter_string.h"
#include "File_io.h"
#include "SHA256.h"
#include "DynamicData_2_lengthBuffer.h"
VOID PROCESS_Request_Real_File(
	PDynamicData input_recv_data,

	PUCHAR* out_send_data,
	ULONG32* out_send_data_size
) {
	if (!input_recv_data || !out_send_data || !out_send_data_size)
		return;
	
	Analysis_Command cmd;

	// 사용자가 요청한 파일 UNICODE_STRING으로 변환
	PCHAR alloc_patha = ExAllocatePoolWithTag(NonPagedPool, input_recv_data->Size + 1, 'tmp');
	if (!alloc_patha)
		return;
	memset(alloc_patha, 0, input_recv_data->Size + 1);
	RtlCopyMemory(alloc_patha, (PCHAR)input_recv_data->Data, input_recv_data->Size);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "사용자요청 파일-> %s \n", alloc_patha);
	ANSI_STRING FILE_PATHA = { 0, };
	RtlInitAnsiString(&FILE_PATHA, alloc_patha);
	UNICODE_STRING FILE_PATHW = { 0, };
	ANSI_to_UNICODE(&FILE_PATHW, FILE_PATHA);

	PUCHAR File_Buffer = NULL;
	ULONG File_Size = 0;
	if (ALL_in_ONE_FILE_IO(&File_Buffer, &File_Size, FILE_PATHW, READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH) != STATUS_SUCCESS) {
		ANSI_to_UNICODE_release(&FILE_PATHW);
		*out_send_data = NULL;
		*out_send_data_size = 0;
		ExFreePoolWithTag(alloc_patha, 'tmp');

		cmd = FAIL;
		Makeing_Analysis_TCP(cmd, NULL, out_send_data, out_send_data_size);

		return;
	}
	else {


		cmd = SUCCESS;
		PDynamicData tmp = CreateDynamicData(File_Buffer, File_Size);
		Makeing_Analysis_TCP(cmd, tmp, out_send_data, out_send_data_size);
		RemoveALLDynamicData(tmp);
	}





	ANSI_to_UNICODE_release(&FILE_PATHW);
	ExFreePoolWithTag(alloc_patha, 'tmp');
	return;
}