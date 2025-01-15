#ifndef Dynamic_Data_Link_Data
#define	Dynamic_Data_Link_Data
#pragma warning(disable: 4996)
#include <ntifs.h>

typedef struct DynamicData{

	PUCHAR Data;
	ULONG32 Size;

	PUCHAR Next_Addr;

}DynamicData, * PDynamicData;

PDynamicData CreateDynamicData(PUCHAR data, ULONG32 size);
PDynamicData AppendDynamicData(PDynamicData current, PUCHAR data, ULONG32 size);

PDynamicData RemoveALLDynamicData(PDynamicData StartAddr);
ULONG32 GetDynamicDataSize(PDynamicData StartAddr);
ULONG32 GetDynamicDataNodeCount(PDynamicData StartAddr, ULONG32* OUtput_ALL_Dynamic_size);

/*
	[ New ] 
*/
BOOLEAN AutoAppend_Source_to_Destination_until_NULL(
	PDynamicData Source_start_node, // in
	PDynamicData* Destination_current_node // out
); // �ڵ����� Source ��带 -> Destination ��忡 ������ ��� ���翬��

// Ư�� ��带 �ٲ�ġ���Ѵ�. ( ���� �����ʹ� �Ҵ������� ) 
BOOLEAN Change_Node_Data(PDynamicData INPUT_NODE, PUCHAR change_data, ULONG32 change_data_size);

// ��� ����� �����͸� PagedPool�� ��ȯ�Ѵ�.( WORK_ITEM������ )
BOOLEAN Convert_ALL_NodeData_2_PagedPool(PDynamicData INPUT_START_NODE, PDynamicData* OUTPUT_converted_Start_Node, KIRQL current_IRQL);

#endif // !Dynamic_Data_Link_Data
