#include "Init_session.h"

#include "query_system_smbios_information.h"
#include "TCP_send_or_Receiver.h"
#include "OS_Version_Check.h"

#include "query_network_info.h"


#include "DynamicData_2_lengthBuffer.h"
NTSTATUS INIT_SESSION() {
	/*
		SMBIOS 1 + 2 + OSVERSION 정보 총 3가지를 하나로 더하여 EDR에게 전달한다. 
		
		
		
		( 여기에서만 !! 길이 기반 규칙을 지키지 않는다. ) 

	*/


	// SMBIOS 정보 쿼리
	PUCHAR SMBIOS_1_Type_DATA = NULL;
	ULONG32 SMBIOS_1_Type_DATA_SIZE = 0;
	PUCHAR SMBIOS_2_Type_DATA = NULL;
	ULONG32 SMBIOS_2_Type_DATA_SIZE = 0;
	NTSTATUS status = Query_SMBIOS_information(&SMBIOS_1_Type_DATA, &SMBIOS_1_Type_DATA_SIZE, &SMBIOS_2_Type_DATA, &SMBIOS_2_Type_DATA_SIZE);
	if (status != STATUS_SUCCESS) {
		goto EXIT;
	}

	
	// 타입 1 과 2 결합
	PUCHAR SMBIOS_DATA = NULL;
	SMBIOS_DATA = ExAllocatePoolWithTag(PagedPool, SMBIOS_1_Type_DATA_SIZE + SMBIOS_2_Type_DATA_SIZE, INIT_SESSION_TAG);
	if (!SMBIOS_DATA) {
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto EXIT;
	}

	memset(SMBIOS_DATA, 0, (SMBIOS_1_Type_DATA_SIZE + SMBIOS_2_Type_DATA_SIZE));
	RtlCopyMemory(SMBIOS_DATA, SMBIOS_1_Type_DATA, SMBIOS_1_Type_DATA_SIZE); // 타입 1 
	RtlCopyMemory(SMBIOS_DATA + SMBIOS_1_Type_DATA_SIZE, SMBIOS_2_Type_DATA, SMBIOS_2_Type_DATA_SIZE); // 타입 1 + 2 

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "전달할 SMBIOS-type1길이: %d @ type2길이: %d\n", SMBIOS_1_Type_DATA_SIZE, SMBIOS_2_Type_DATA_SIZE);

	// SMBIOS 
	PDynamicData send_data = CreateDynamicData(SMBIOS_DATA, SMBIOS_1_Type_DATA_SIZE + SMBIOS_2_Type_DATA_SIZE);
	PDynamicData current_send_data = send_data;

	// OS_INFO
	PUCHAR os_info = NULL;
	ULONG32 os_info_str_len = 0;
	OS_Version_Checker(&os_info);
	os_info_str_len = (ULONG32)strlen((PCHAR)os_info);

	current_send_data = AppendDynamicData(current_send_data, os_info, os_info_str_len);
	OS_info_free(os_info); // 문자열 할당 해제

	// MAC주소와 IPv4 둘다
	PUCHAR MacAddressA = NULL;
	ULONG32 MacAddressA_len = 0;
	PCHAR IPv4Address = NULL;
	ULONG32 IPv4Address_len = 0;
	Get_Local_Network_info(&MacAddressA, &MacAddressA_len , &IPv4Address, &IPv4Address_len);

	current_send_data = AppendDynamicData(current_send_data, (PUCHAR)IPv4Address, IPv4Address_len - 1);
	current_send_data = AppendDynamicData(current_send_data, MacAddressA, MacAddressA_len-1);
	

	FREE_Get_LOCAL_network_info((PVOID)MacAddressA, (PVOID)IPv4Address);

	PUCHAR out_send_data = NULL;
	ULONG32 out_send_data_size = 0;
	Makeing_Analysis_TCP(0, (PDynamicData)send_data, &out_send_data, &out_send_data_size);
	RemoveALLDynamicData(send_data);

	// 초기데이터 송신
	status = SEND_TCP_DATA(out_send_data, out_send_data_size, TCP_DATA_SEND);
	if ( status != STATUS_SUCCESS) {

		goto EXIT_1;
	}
	
	goto EXIT_1;



EXIT_1:
	{
		if(out_send_data)
			ExFreePoolWithTag(out_send_data, session_alloc_tag); // 둘 결합한 데이터 해제

		goto EXIT;
	}
EXIT:
	{
		Release_Query_SMBIOS_information(&SMBIOS_1_Type_DATA, &SMBIOS_2_Type_DATA);
		return status;
	}
}
