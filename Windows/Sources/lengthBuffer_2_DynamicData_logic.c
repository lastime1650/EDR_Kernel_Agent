#include "lengthBuffer_2_DynamicData.h"

VOID lenBuffer_2_DynamicData(
	PUCHAR in_Data,
	ULONG32 in_Data_Length,

	ULONG32* out_Command,
	PDynamicData* out_Dynamic_data
) {
	if (!out_Dynamic_data || !out_Command ) 
		return;

	PDynamicData Start_Address = NULL;
	PDynamicData Current_Address = NULL;

	ULONG32 current_size = 0;


	*out_Command = *(PULONG32)in_Data; // ������ ����
	in_Data += sizeof(ULONG32); // ������ �̵�
	current_size += sizeof(ULONG32); // ���� ������ ������ 

	// _END�϶����� ���ѹݺ�
	while (1) {

		// Ż���� ���� ���̰���
		if (current_size >= in_Data_Length) break;


		if (memcmp(in_Data, "_END", sizeof("_END") - 1) == 0) {
			return;
		}
		else {

			// ��������
			ULONG32 Length = *(PULONG32)in_Data;
			in_Data += sizeof(ULONG32); // ������ �̵�
			current_size += sizeof(ULONG32);

			// ����������
			PUCHAR Alloc_Data = ExAllocatePool(NonPagedPool, Length);
			if (!Alloc_Data) {
				if (Start_Address) {
					RemoveALLDynamicData(Start_Address);
				}
				*out_Dynamic_data = NULL;

				return;
			}

			memcpy(Alloc_Data, in_Data, Length);
			in_Data += Length; // ������ �̵�
			current_size += Length;

			// ���Ḯ��Ʈ�� ����
			if (!Start_Address) {
				Start_Address = CreateDynamicData(Alloc_Data, Length);
				*out_Dynamic_data = Start_Address;

				Current_Address = Start_Address;
			}
			else {
				Current_Address = AppendDynamicData(Current_Address, Alloc_Data, Length);
			}

			ExFreePool(Alloc_Data);
		}

	}
	return;
}