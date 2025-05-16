#include "SID.h"
#pragma warning(disable:4996)


NTSTATUS Get_User_SID_with_Alloc(PSID* out_sid, Users_for_a_SID choice_the_user) {
	if (out_sid == NULL) return STATUS_MEMORY_NOT_ALLOCATED;
	*out_sid = NULL;

	ULONG32 needSidSize = RtlLengthRequiredSid(1); // SubAuthority 개수가 1개

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
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RtlInitializeSid 실패 %p\n", status);
		ExFreePoolWithTag(SID,'SIDx');
		return status;
	}
		

	// 이후 SID는 S-1-5  까지 완성됨
	

	switch (choice_the_user) {
	case SYSTEM:
	{
		// 뒤에 "18"을 더해서 S-1-5-18 완성시키기
		// RtlSubAuthoritySid는 SID에 붙일 '주소를 반환'하는데, 즉시 *를 붙여 바로 그 위치에 참조하여 값을 넣음
		*( RtlSubAuthoritySid(SID,0 ) ) = SECURITY_LOCAL_SYSTEM_RID;
		*out_sid = SID;
		break;
	}
	case NETWORK_SERVICE:
	{
		// 뒤에 "20"을 더해서 S-1-5-20 완성시키기
		// RtlSubAuthoritySid는 SID에 붙일 '주소를 반환'하는데, 즉시 *를 붙여 바로 그 위치에 참조하여 값을 넣음
		* ( RtlSubAuthoritySid(
			SID,
			0 // 붙일 인덱스 값
		) ) = SECURITY_NETWORK_SERVICE_RID;
		*out_sid = SID;
		break;
	}
	case LOCAL_SERVICE:
	{
		// 뒤에 "19"을 더해서 S-1-5-19 완성시키기
		// RtlSubAuthoritySid는 SID에 붙일 '주소를 반환'하는데, 즉시 *를 붙여 바로 그 위치에 참조하여 값을 넣음
		*(RtlSubAuthoritySid(
			SID,
			0 // 붙일 인덱스 값
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