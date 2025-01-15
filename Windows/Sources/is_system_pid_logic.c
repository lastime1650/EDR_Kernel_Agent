#include "is_system_pid.h"

#include "API_Loader.h"
#include "SID.h"// 사용자 계정 체크용
BOOLEAN Is_it_System_Process(HANDLE PID ) {
	
	CHAR current_irql = KeGetCurrentIrql();

	NTSTATUS status;
	BOOLEAN return_bool = TRUE;

	if (current_irql == DISPATCH_LEVEL) {
		return_bool = FALSE;
		goto LABEL_0;
	}

	PEPROCESS Process = NULL;
	PsLookupProcessByProcessId(PID, &Process);
	if (Process == NULL) {
		return_bool = TRUE;
		goto LABEL_0;
	}


	BOOLEAN is_system = PsIsSystemProcess(Process);
	if (is_system ) {
		return_bool = is_system;
		goto LABEL_1;
	}

	if (current_irql != PASSIVE_LEVEL  )
	{
		return_bool = FALSE;
		goto LABEL_1;
	}
		

	/*
		아래부터는 SID 기반의 [2차전]
		PASSIVE_LEVEL이 아니면 BSOD 발생
	*/


	// [ New ]이래도 시스템 관련 프로세스가 로그에 쌓이는 경우가 있어, 관련한 모든 시스템 프로세스는 FALSE로 반환하도록 함.

	PACCESS_TOKEN process_token = PsReferencePrimaryToken(Process); // 1. 참조 수 증가 ( ACCESS_TOKEN을 얻기 위해 )
	if (process_token == NULL) {
		return_bool = FALSE;
		goto LABEL_1;
	}

	TOKEN_USER* process_user = NULL;
	status = SeQueryInformationToken(
		process_token,
		TokenUser, // 이 프로세스를 '실행'한 사용자
		//TokenOwner, // 이 프로세스를 '생성'한 사용자
		&process_user
	);
	if (!process_user)
		goto LABEL_2;
	//PsDereferencePrimaryToken(Process); // 참조 감소

	if (status != STATUS_SUCCESS) {
		return_bool = FALSE;
		goto LABEL_2;
	}

	PSID sid = NULL;
	/*
		SYSTEM 계정인가?
	*/
	status = Get_User_SID_with_Alloc(&sid, SYSTEM);
	if (sid == NULL) {
		return_bool = FALSE;
		goto LABEL_3;
	}

	// [1/2] 검사
	BOOLEAN is_same = RtlEqualSid(process_user->User.Sid, sid);
	SID_Release(&sid);
	if (is_same == TRUE) {
		return_bool = TRUE;
		goto LABEL_3;
	}
	

	//


	/*
		Network_SERVICE 계정인가?
	*/
	sid = NULL;
	status = Get_User_SID_with_Alloc(&sid, NETWORK_SERVICE);
	if (sid == NULL) {
		return_bool = FALSE;
		goto LABEL_3;
	}

	// 검사
	is_same = RtlEqualSid(process_user->User.Sid, sid);
	SID_Release(&sid);
	if (is_same == TRUE) {
		return_bool = TRUE;
		goto LABEL_3;
	}

	//

	/*
		local service?
	*/
	sid = NULL;
	status = Get_User_SID_with_Alloc(&sid, LOCAL_SERVICE);
	if (sid == NULL) {
		return_bool = FALSE;
		goto LABEL_3;
	}

	// 검사
	is_same = RtlEqualSid(process_user->User.Sid, sid);
	SID_Release(&sid);
	if (is_same == TRUE) {
		return_bool = TRUE;
		goto LABEL_3;
	}
	
	return_bool = FALSE;
	goto LABEL_3;




LABEL_3:
	{
		ExFreePool(process_user); // MSDN에서 하라는 데로 'SeQueryInformationToken' 는 페이징공간에 할당해서 반환해줌
		goto LABEL_2;
	}
LABEL_2:
	{
		PsDereferencePrimaryToken(Process); // 참조 감소
		goto LABEL_0;
	}
LABEL_1:
	{
		ObDereferenceObject(Process);
		goto LABEL_0;
	}
LABEL_0:
	{
		return return_bool;
	}
}