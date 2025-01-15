#ifndef RESPONSE_FILE_H
#define RESPONSE_FILE_H

#include <ntifs.h>
#include "DynamicData_Linked_list.h"
#include "SHA256.h"

typedef PDynamicData FileResponseData;


/*
	*************[ �˾ƾ� �� �� ]*************************

	PUCHAR�� FileResponse_Struct ������ ����ü

*/
typedef struct FileResponse_Struct {

	CHAR SHA256[SHA256_String_Byte_Length];
	ULONG32 File_Size;

}FileResponse_Struct, *PFileResponse_Struct;

extern FileResponseData File_Response_Start_Node_Address;
extern FileResponseData File_Response_Current_Node_Address;

BOOLEAN is_same_check(PUNICODE_STRING input_filepath, ULONG32 File_SIze); // ����
BOOLEAN Append_File_Response_Data(PCHAR SHA256, ULONG32 File_SIze, FileResponseData* opt_output_start_node); // ���� ��� �߰� ( ��, �ߺ� ���� )
BOOLEAN Remove_File_Reponse_Data(PCHAR SHA256, FileResponseData* opt_output_start_node); // ���� ��� ���� ( 1���� )
BOOLEAN is_exist_File_Response_Data(PCHAR SHA256); // ���� ��� Ȯ��


VOID Print_File_Response_Data_Nodes();

#endif