#include "PROCESS_Request_File_Response.h"
#include "Analysis_enum.h"
#include "DynamicData_2_lengthBuffer.h"

VOID PROCESS_Request_File_Response(PDynamicData Parsed_data, PUCHAR* out_send_data, ULONG32* out_send_data_size, BOOLEAN is_remove) {
	
	//

	PDynamicData current = Parsed_data; // [0] 
	PCHAR SHA256_addr = (PCHAR)current->Data;


	current = (PDynamicData)Parsed_data->Next_Addr; // [1]
	ULONG32 FILE_SIZE = *(ULONG32*)current->Data;
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " 파일 차단 SHA256 받음 -> %64s / 사이즈 -> %lu \n", SHA256_addr, FILE_SIZE);
	//
	Analysis_Command cmd;

	FileResponseData response_data = NULL;

	if(!is_remove){
		if (Append_File_Response_Data(SHA256_addr, FILE_SIZE, &response_data)) {
			// 추가 성공


			cmd = SUCCESS;

		}
		else {
			// 추가 실패
			cmd = FAIL;
		}
	}
	else {
		if (Remove_File_Reponse_Data(SHA256_addr, &response_data)) {
			// 추가 성공


			cmd = SUCCESS;

		}
		else {
			// 추가 실패
			cmd = FAIL;
		}
	}
	


	*out_send_data = NULL;
	*out_send_data_size = 0;
	Makeing_Analysis_TCP(cmd, (PDynamicData)response_data, out_send_data, out_send_data_size);


	return;

}