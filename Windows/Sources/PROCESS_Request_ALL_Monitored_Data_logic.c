#include "PROCESS_Request_ALL_Monitored_Data.h"

#include "Analysis_enum.h"
#include "DynamicData_2_lengthBuffer.h"
VOID PROCESS_Request_ALL_Monitored_Data(
	PUCHAR* out_send_data,
	ULONG32* out_send_data_size
) {

	if (!out_send_data || !out_send_data_size)
		return;
	
	*out_send_data = NULL;
	*out_send_data_size = 0;

	// 버퍼 가져오기
	PUCHAR buff = NULL;
	ULONG32 buff_size = 0;
	Output_lenBuff(
		&buff,
		&buff_size
	);

	// 
	Analysis_Command cmd;
	if (buff == NULL) {
		cmd = FAIL;
		// 실패 버퍼 생성
		Makeing_Analysis_TCP(cmd, NULL, out_send_data, out_send_data_size);
	}
	else {
		cmd = SUCCESS;
		*out_send_data = buff;
		*out_send_data_size = buff_size;
	}


	return;
}