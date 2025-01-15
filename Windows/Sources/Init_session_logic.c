#include "Init_session.h"

#include "query_system_smbios_information.h"
#include "TCP_send_or_Receiver.h"
NTSTATUS INIT_SESSION() {
	/*
		SMBIOS 1 + 2 를 하나로 더하여 EDR에게 전달한다. 
		
		
		
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
	PUCHAR Received_Data = NULL;
	Received_Data = ExAllocatePoolWithTag(PagedPool, SMBIOS_1_Type_DATA_SIZE + SMBIOS_2_Type_DATA_SIZE, INIT_SESSION_TAG);
	if (!Received_Data) {
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto EXIT;
	}

	memset(Received_Data, 0, (SMBIOS_1_Type_DATA_SIZE + SMBIOS_2_Type_DATA_SIZE));
	RtlCopyMemory(Received_Data, SMBIOS_1_Type_DATA, SMBIOS_1_Type_DATA_SIZE); // 타입 1 
	RtlCopyMemory(Received_Data + SMBIOS_1_Type_DATA_SIZE, SMBIOS_2_Type_DATA, SMBIOS_2_Type_DATA_SIZE); // 타입 1 + 2 

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "전달할 SMBIOS-type1길이: %d @ type2길이: %d\n", SMBIOS_1_Type_DATA_SIZE, SMBIOS_2_Type_DATA_SIZE);
	// 초기데이터 송신
	status = SEND_TCP_DATA(Received_Data, (ULONG32)(SMBIOS_1_Type_DATA_SIZE + SMBIOS_2_Type_DATA_SIZE), TCP_DATA_SEND);
	if ( status != STATUS_SUCCESS) {

		goto EXIT_1;
	}
	
	goto EXIT_1;



EXIT_1:
	{
		if(Received_Data)
			ExFreePoolWithTag(Received_Data, INIT_SESSION_TAG); // 둘 결합한 데이터 해제

		goto EXIT;
	}
EXIT:
	{
		Release_Query_SMBIOS_information(&SMBIOS_1_Type_DATA, &SMBIOS_2_Type_DATA);
		return status;
	}
}
