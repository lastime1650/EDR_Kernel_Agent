#ifndef RESPONSE_FILE_H
#define RESPONSE_FILE_H

#include <ntifs.h>
#include "DynamicData_Linked_list.h"
#include "SHA256.h"

typedef PDynamicData FileResponseData;


/*
	*************[ 알아야 할 것 ]*************************

	PUCHAR은 FileResponse_Struct 포인터 구조체

*/
typedef struct FileResponse_Struct {

	CHAR SHA256[SHA256_String_Byte_Length];
	ULONG32 File_Size;

}FileResponse_Struct, *PFileResponse_Struct;

extern FileResponseData File_Response_Start_Node_Address;
extern FileResponseData File_Response_Current_Node_Address;

BOOLEAN is_same_check(PUNICODE_STRING input_filepath, ULONG32 File_SIze); // 차단
BOOLEAN Append_File_Response_Data(PCHAR SHA256, ULONG32 File_SIze, FileResponseData* opt_output_start_node); // 차단 목록 추가 ( 단, 중복 금지 )
BOOLEAN Remove_File_Reponse_Data(PCHAR SHA256, FileResponseData* opt_output_start_node); // 차단 목록 제거 ( 1개씩 )
BOOLEAN is_exist_File_Response_Data(PCHAR SHA256); // 차단 목록 확인


VOID Print_File_Response_Data_Nodes();

#endif