#include "DynamicData_Linked_list.h"


PDynamicData CreateDynamicData(PUCHAR data, ULONG32 size) {

	if (size == 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "CreateDynamicData 길이 0\n");
		return NULL;
	}

	PDynamicData NewData = (PDynamicData)ExAllocatePoolWithTag(NonPagedPool, sizeof(DynamicData), 'NODE');
	if (NewData == NULL)
	{
		return NULL;
	}

	if (KeGetCurrentIrql() != DISPATCH_LEVEL) {
		NewData->Data = (PUCHAR)ExAllocatePoolWithTag(PagedPool, size, 'BUFF');
	}
	else {
		NewData->Data = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, size, 'BUFF');
		
	}
	
	if (NewData->Data == NULL) {
		ExFreePoolWithTag(NewData, 'NODE');
		return NULL;
	}
	memset(NewData->Data, 0, size);

	RtlCopyMemory(NewData->Data, data, size);
	NewData->Size = size;
	NewData->Next_Addr = NULL;


	return NewData;
}

PDynamicData AppendDynamicData(PDynamicData current, PUCHAR data, ULONG32 size){
	if (current == NULL) return NULL;
	PDynamicData NewData = CreateDynamicData(data, size);
	if (NewData == NULL) return NULL;

	current->Next_Addr = (PUCHAR)NewData;

	return NewData;
}

// [NEW]
BOOLEAN AutoAppend_Source_to_Destination_until_NULL(PDynamicData Source_start_node, PDynamicData* Destination_current_node) {
	if (Source_start_node == NULL || Destination_current_node  == NULL || *Destination_current_node == NULL) return FALSE;

	PDynamicData current_source = Source_start_node;
	PDynamicData Updating_Destination = *Destination_current_node;
	while (current_source != NULL) {

		// To Destination Node 
		Updating_Destination = AppendDynamicData(Updating_Destination, current_source->Data, current_source->Size);

		current_source = (PDynamicData)current_source->Next_Addr;
	}

	// Update 중간에 노드를 계속 삽입했던 부분에 대해서 current주소를 갱신!
	*Destination_current_node = Updating_Destination;

	return TRUE;
}

PDynamicData RemoveALLDynamicData(PDynamicData StartAddr) {
	if (!StartAddr) return NULL;
	PDynamicData current = StartAddr;

	while (current != NULL) {
		PDynamicData tmp_next_node = (PDynamicData)current->Next_Addr;
		
		ExFreePoolWithTag(current->Data,'BUFF');
		ExFreePoolWithTag(current,'NODE');

		current = (PDynamicData)tmp_next_node;

	}

	
	return NULL;
}

ULONG32 GetDynamicDataSize(PDynamicData StartAddr) {
	PDynamicData current = StartAddr;
	ULONG32 total_size = 0;

	while (current != NULL) {
		total_size += current->Size;
		current = (PDynamicData)current->Next_Addr;
	}

	return total_size;
}


ULONG32 GetDynamicDataNodeCount(PDynamicData StartAddr, ULONG32* OUtput_ALL_Dynamic_size) {
	PDynamicData current = StartAddr;
	ULONG32 count = 0;

	while (current != NULL) {
		if (OUtput_ALL_Dynamic_size) {
			*OUtput_ALL_Dynamic_size += current->Size;
		}
		count++;
		current = (PDynamicData)current->Next_Addr;
	}

	return count;
}


// 특정 노드를 바꿔치기한다. ( 이전 데이터는 할당해제됨 ) 
BOOLEAN Change_Node_Data(PDynamicData INPUT_NODE, PUCHAR change_data, ULONG32 change_data_size) {
	if (!INPUT_NODE || !change_data || (change_data_size < 1))
		return FALSE;
	
	// 미리 할당할 데이터 할당
	PUCHAR new_data = ExAllocatePoolWithTag(PagedPool, change_data_size, 'BUFF');
	if (!new_data)
		return FALSE;

	// 이전 값 할당해제
	ExFreePoolWithTag(INPUT_NODE->Data, 'BUFF');
	INPUT_NODE->Size = 0;

	// 새로 할당
	INPUT_NODE->Data = new_data;
	INPUT_NODE->Size = change_data_size;

	// 데이터 삽입
	RtlCopyMemory(INPUT_NODE->Data, change_data, change_data_size);

	return TRUE;
}



// 모든 노드의 데이터를 PagedPool로 변환한다.( WORK_ITEM전용형 )
BOOLEAN Convert_ALL_NodeData_2_PagedPool(PDynamicData INPUT_START_NODE, PDynamicData* OUTPUT_converted_Start_Node, KIRQL current_IRQL) {
	if (!INPUT_START_NODE || current_IRQL != PASSIVE_LEVEL)
		return FALSE;

	PDynamicData converted_Start_Node = NULL;
	PDynamicData converted_Current_Node = NULL;

	PDynamicData current_free_node = INPUT_START_NODE;
	while (current_free_node != NULL) {
		
		PDynamicData Remember_will_free_NextNode = (PDynamicData)current_free_node->Next_Addr;

		// COPY
		if (converted_Start_Node == NULL) {
			converted_Start_Node = CreateDynamicData(current_free_node->Data, current_free_node->Size);
			converted_Current_Node = converted_Start_Node;
			*OUTPUT_converted_Start_Node = converted_Start_Node; // OUTPUT
		}
		else {
			converted_Current_Node = AppendDynamicData(converted_Current_Node, current_free_node->Data, current_free_node->Size);
		}

		// FREE
		//ExFreePoolWithTag(current_free_node->Data, 'BUFF');
		//ExFreePoolWithTag(current_free_node, 'NODE');

		current_free_node = Remember_will_free_NextNode;
	}


	return TRUE;
}

