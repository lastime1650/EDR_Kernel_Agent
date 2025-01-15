#ifndef Response_Process_H
#define Response_Process_H

#include "ntifs.h"
#include "DynamicData_Linked_list.h"

#define HASH_LEN 64 // PUCHAR �ؽñ��� null ���ʿ�


typedef PDynamicData ProcessResponseData;


/*
	*************[ �˾ƾ� �� �� ]*************************

	PUCHAR�� ProcessResponse_Struct ������ ����ü

*/
typedef struct ProcessResponse_Struct {
	CHAR SHA256[64];
	ULONG32 FILE_SIZE;
}ProcessResponse_Struct, *PProcessResponse_Struct;

extern ProcessResponseData Process_Response_Start_Node_Address;
extern ProcessResponseData Process_Response_Current_Node_Address;

// 1. ���� ũ�� ��ġ �˻� -> 2. SHA256üũ -> 3. True�� �� ����
BOOLEAN Check_Process_Response(HANDLE* pid); // �����ϱ� �� ���� - ���� ������ ����

BOOLEAN Append_Process_Response_Data(PCHAR SHA256, ULONG32 FILE_SIZE, ProcessResponseData* opt_output_start_node); // ���� ��� �߰� ( ��, �ߺ� ���� )
BOOLEAN Remove_Process_Reponse_Data(PCHAR SHA256, ProcessResponseData* opt_output_start_node); // ���� ��� ���� ( 1���� )
BOOLEAN is_exist_Process_Response_Data(PCHAR SHA256); // ���� ��� Ȯ�� ��


VOID Print_Process_Response_Data_Nodes();
#endif