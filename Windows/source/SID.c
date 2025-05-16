#include "SID.h"
#pragma warning(disable:4996)


NTSTATUS Get_User_SID_with_Alloc(PSID* out_sid, Users_for_a_SID choice_the_user) {
	if (out_sid == NULL) return STATUS_MEMORY_NOT_ALLOCATED;
	*out_sid = NULL;

	ULONG32 needSidSize = RtlLengthRequiredSid(1); // SubAuthority ������ 1��

	//PSID SID = (PSID)ExAllocatePoolWithTag(NonPagedPool, SECURITY_MAX_SID_SIZE, 'SIDx');
	PSID SID = (PSID)ExAllocatePoolWithTag(NonPagedPool, needSidSize, 'SIDx');
	if (SID == NULL) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	
	NTSTATUS status; 

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

	status = RtlInitializeSid(
		SID,
		&NtAuthority,
		1 // S-1-5  .. 
	);
	if (status != STATUS_SUCCESS) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RtlInitializeSid ���� %p\n", status);
		ExFreePoolWithTag(SID,'SIDx');
		return status;
	}
		

	// ���� SID�� S-1-5  ���� �ϼ���
	

	switch (choice_the_user) {
	case SYSTEM:
	{
		// �ڿ� "18"�� ���ؼ� S-1-5-18 �ϼ���Ű��
		// RtlSubAuthoritySid�� SID�� ���� '�ּҸ� ��ȯ'�ϴµ�, ��� *�� �ٿ� �ٷ� �� ��ġ�� �����Ͽ� ���� ����
		*( RtlSubAuthoritySid(SID,0 ) ) = SECURITY_LOCAL_SYSTEM_RID;
		*out_sid = SID;
		break;
	}
	case NETWORK_SERVICE:
	{
		// �ڿ� "20"�� ���ؼ� S-1-5-20 �ϼ���Ű��
		// RtlSubAuthoritySid�� SID�� ���� '�ּҸ� ��ȯ'�ϴµ�, ��� *�� �ٿ� �ٷ� �� ��ġ�� �����Ͽ� ���� ����
		* ( RtlSubAuthoritySid(
			SID,
			0 // ���� �ε��� ��
		) ) = SECURITY_NETWORK_SERVICE_RID;
		*out_sid = SID;
		break;
	}
	case LOCAL_SERVICE:
	{
		// �ڿ� "19"�� ���ؼ� S-1-5-19 �ϼ���Ű��
		// RtlSubAuthoritySid�� SID�� ���� '�ּҸ� ��ȯ'�ϴµ�, ��� *�� �ٿ� �ٷ� �� ��ġ�� �����Ͽ� ���� ����
		*(RtlSubAuthoritySid(
			SID,
			0 // ���� �ε��� ��
		)) = SECURITY_LOCAL_SERVICE_RID;
		*out_sid = SID;
		break;
	}
	default:
		*out_sid = NULL;
		ExFreePoolWithTag(SID, 'SIDx');
		return STATUS_UNSUCCESSFUL;
	}
	/*
	// COPY 
	ULONG32 need_size = RtlLengthSid(SID);
	*out_sid = (PSID)ExAllocatePoolWithTag(NonPagedPool, need_size, 'SIDx');

	RtlCopySid(need_size, *out_sid, SID); // copy 

	ExFreePoolWithTag(SID, 'cp');
	*/
	return status;
}

VOID SID_Release(PSID* release_addr) {
	ExFreePoolWithTag(*release_addr, 'SIDx');
	*release_addr = NULL;
}