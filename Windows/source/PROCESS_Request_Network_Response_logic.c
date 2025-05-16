#include "PROCESS_Request_Network_Response.h"
#include "Analysis_enum.h"
#include "DynamicData_2_lengthBuffer.h"

VOID PROCESS_Request_Network_Response(PDynamicData parsed_data, PUCHAR* out_send_data, ULONG32* out_send_data_size, BOOLEAN is_remove) {

	PCHAR RemoteIP = ExAllocatePoolWithTag(NonPagedPool, parsed_data->Size + 1, 'tmpA');
	memset(RemoteIP, 0, parsed_data->Size + 1);
	RtlCopyMemory(RemoteIP, parsed_data->Data, parsed_data->Size);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "네트워크 차단요청 -> %s / 길이 %d / strlen() 한 후: %d \n", RemoteIP, parsed_data->Size, strlen(RemoteIP));
	Analysis_Command cmd;
	NetworkResponseData response_data = NULL;


	if (!is_remove) {
		if (Append_Network_Response_Data(RemoteIP, &response_data)) {
			cmd = SUCCESS;
		}
		else {
			cmd = FAIL;
		}
	}
	else {
		if(Remove_Networks_Reponse_Data(RemoteIP, &response_data))
		{
			cmd = SUCCESS;
		}
		else {
			cmd = FAIL;
		}
	}
	

	*out_send_data = NULL;
	*out_send_data_size = 0;
	Makeing_Analysis_TCP(cmd, (PDynamicData)response_data, out_send_data, out_send_data_size);

	ExFreePoolWithTag(RemoteIP, 'tmpA');
	return;

}