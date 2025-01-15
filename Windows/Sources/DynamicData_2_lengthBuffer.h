#ifndef Dyn_2_BUff

#define Dyn_2_BUff

#include <ntifs.h>

/*
	���Ḯ��Ʈ�� �ϳ��� "���̱��"���� ��ȯ�Ѵ�. 
*/

#define Head 4
#define LENGTH_MAX_BYTE_NUM 4
#define Tail "_END"

#define LEN_BUFF_TAG 'LBT_'

#include "Analysis_enum.h"
#include "DynamicData_Linked_list.h"
typedef struct mover {

	HANDLE PID;

	Analysis_Command cmd;

	PDynamicData start_node; //( [Update]:: �� �۾��� Work-Item�� ��ϵ� �����忡�� �̸� ���̱�� �����͸� �����ϵ��� �����ȴ�. )

	PCHAR timestamp;

}mover, *Pmover;

#define mover_tag 'MmTg'

extern PUCHAR SEND_DATA;
extern ULONG32 SEND_DATA_SIZE;

VOID Dyn_2_lenBuff(Pmover in_parameter);


#include "session_comunicate.h"
VOID Output_lenBuff(PUCHAR* output_buffer, ULONG32* output_size);

// session_comunicate
BOOLEAN Makeing_Analysis_TCP(ULONG32 Command, PDynamicData Input_Dynamic_Data, PUCHAR* OUTPUT, ULONG32* OUTPUT_size);
#endif 