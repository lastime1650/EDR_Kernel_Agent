#include "is_system_pid.h"

#include "API_Loader.h"
#include "SID.h"// ����� ���� üũ��
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
		�Ʒ����ʹ� SID ����� [2����]
		PASSIVE_LEVEL�� �ƴϸ� BSOD �߻�
	*/


	// [ New ]�̷��� �ý��� ���� ���μ����� �α׿� ���̴� ��찡 �־�, ������ ��� �ý��� ���μ����� FALSE�� ��ȯ�ϵ��� ��.

	PACCESS_TOKEN process_token = PsReferencePrimaryToken(Process); // 1. ���� �� ���� ( ACCESS_TOKEN�� ��� ���� )
	if (process_token == NULL) {
		return_bool = FALSE;
		goto LABEL_1;
	}

	TOKEN_USER* process_user = NULL;
	status = SeQueryInformationToken(
		process_token,
		TokenUser, // �� ���μ����� '����'�� �����
		//TokenOwner, // �� ���μ����� '����'�� �����
		&process_user
	);
	if (!process_user)
		goto LABEL_2;
	//PsDereferencePrimaryToken(Process); // ���� ����

	if (status != STATUS_SUCCESS) {
		return_bool = FALSE;
		goto LABEL_2;
	}

	PSID sid = NULL;
	/*
		SYSTEM �����ΰ�?
	*/
	status = Get_User_SID_with_Alloc(&sid, SYSTEM);
	if (sid == NULL) {
		return_bool = FALSE;
		goto LABEL_3;
	}

	// [1/2] �˻�
	BOOLEAN is_same = RtlEqualSid(process_user->User.Sid, sid);
	SID_Release(&sid);
	if (is_same == TRUE) {
		return_bool = TRUE;
		goto LABEL_3;
	}
	

	//


	/*
		Network_SERVICE �����ΰ�?
	*/
	sid = NULL;
	status = Get_User_SID_with_Alloc(&sid, NETWORK_SERVICE);
	if (sid == NULL) {
		return_bool = FALSE;
		goto LABEL_3;
	}

	// �˻�
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

	// �˻�
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
		ExFreePool(process_user); // MSDN���� �϶�� ���� 'SeQueryInformationToken' �� ����¡������ �Ҵ��ؼ� ��ȯ����
		goto LABEL_2;
	}
LABEL_2:
	{
		PsDereferencePrimaryToken(Process); // ���� ����
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