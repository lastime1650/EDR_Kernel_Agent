#include "Init_session.h"

#include "query_system_smbios_information.h"
#include "TCP_send_or_Receiver.h"
NTSTATUS INIT_SESSION() {
	/*
		SMBIOS 1 + 2 �� �ϳ��� ���Ͽ� EDR���� �����Ѵ�. 
		
		
		
		( ���⿡���� !! ���� ��� ��Ģ�� ��Ű�� �ʴ´�. ) 

	*/


	// SMBIOS ���� ����
	PUCHAR SMBIOS_1_Type_DATA = NULL;
	ULONG32 SMBIOS_1_Type_DATA_SIZE = 0;
	PUCHAR SMBIOS_2_Type_DATA = NULL;
	ULONG32 SMBIOS_2_Type_DATA_SIZE = 0;
	NTSTATUS status = Query_SMBIOS_information(&SMBIOS_1_Type_DATA, &SMBIOS_1_Type_DATA_SIZE, &SMBIOS_2_Type_DATA, &SMBIOS_2_Type_DATA_SIZE);
	if (status != STATUS_SUCCESS) {
		goto EXIT;
	}

	
	// Ÿ�� 1 �� 2 ����
	PUCHAR Received_Data = NULL;
	Received_Data = ExAllocatePoolWithTag(PagedPool, SMBIOS_1_Type_DATA_SIZE + SMBIOS_2_Type_DATA_SIZE, INIT_SESSION_TAG);
	if (!Received_Data) {
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto EXIT;
	}

	memset(Received_Data, 0, (SMBIOS_1_Type_DATA_SIZE + SMBIOS_2_Type_DATA_SIZE));
	RtlCopyMemory(Received_Data, SMBIOS_1_Type_DATA, SMBIOS_1_Type_DATA_SIZE); // Ÿ�� 1 
	RtlCopyMemory(Received_Data + SMBIOS_1_Type_DATA_SIZE, SMBIOS_2_Type_DATA, SMBIOS_2_Type_DATA_SIZE); // Ÿ�� 1 + 2 

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "������ SMBIOS-type1����: %d @ type2����: %d\n", SMBIOS_1_Type_DATA_SIZE, SMBIOS_2_Type_DATA_SIZE);
	// �ʱⵥ���� �۽�
	status = SEND_TCP_DATA(Received_Data, (ULONG32)(SMBIOS_1_Type_DATA_SIZE + SMBIOS_2_Type_DATA_SIZE), TCP_DATA_SEND);
	if ( status != STATUS_SUCCESS) {

		goto EXIT_1;
	}
	
	goto EXIT_1;



EXIT_1:
	{
		if(Received_Data)
			ExFreePoolWithTag(Received_Data, INIT_SESSION_TAG); // �� ������ ������ ����

		goto EXIT;
	}
EXIT:
	{
		Release_Query_SMBIOS_information(&SMBIOS_1_Type_DATA, &SMBIOS_2_Type_DATA);
		return status;
	}
}
