#ifndef Response_Process_H
#define Response_Process_H

#include "ntifs.h"
#include "DynamicData_Linked_list.h"

#define HASH_LEN 64 // PUCHAR 해시길이 null 불필요


typedef PDynamicData ProcessResponseData;


/*
	*************[ 알아야 할 것 ]*************************

	PUCHAR은 ProcessResponse_Struct 포인터 구조체

*/
typedef struct ProcessResponse_Struct {
	CHAR SHA256[64];
	ULONG32 FILE_SIZE;
}ProcessResponse_Struct, *PProcessResponse_Struct;

extern ProcessResponseData Process_Response_Start_Node_Address;
extern ProcessResponseData Process_Response_Current_Node_Address;

// 1. 파일 크기 일치 검사 -> 2. SHA256체크 -> 3. True일 때 차단
BOOLEAN Check_Process_Response(HANDLE* pid); // 차단하기 전 검증 - 파일 사이즈 기준

BOOLEAN Append_Process_Response_Data(PCHAR SHA256, ULONG32 FILE_SIZE, ProcessResponseData* opt_output_start_node); // 차단 목록 추가 ( 단, 중복 금지 )
BOOLEAN Remove_Process_Reponse_Data(PCHAR SHA256, ProcessResponseData* opt_output_start_node); // 차단 목록 제거 ( 1개씩 )
BOOLEAN is_exist_Process_Response_Data(PCHAR SHA256); // 차단 목록 확인 용


VOID Print_Process_Response_Data_Nodes();
#endif