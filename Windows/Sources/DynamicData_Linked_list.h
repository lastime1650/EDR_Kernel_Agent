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
); // 자동으로 Source 노드를 -> Destination 노드에 데이터 노드 복사연결

// 특정 노드를 바꿔치기한다. ( 이전 데이터는 할당해제됨 ) 
BOOLEAN Change_Node_Data(PDynamicData INPUT_NODE, PUCHAR change_data, ULONG32 change_data_size);

// 모든 노드의 데이터를 PagedPool로 변환한다.( WORK_ITEM전용형 )
BOOLEAN Convert_ALL_NodeData_2_PagedPool(PDynamicData INPUT_START_NODE, PDynamicData* OUTPUT_converted_Start_Node, KIRQL current_IRQL);

#endif // !Dynamic_Data_Link_Data
