#ifndef WORKITEM_STRUCT

#define WORKITEM_STRUCT

#include <ntifs.h>

#define WORK_ITEM_TAG 'WITg'


typedef struct WORK_CONTEXT{

	PVOID context; // �߰����� �Ű����� ������
	PKSTART_ROUTINE startroutine; // work �ȿ��� ȣ���� ������ �Լ�

}WORK_CONTEXT, * PWORK_CONTEXT;

typedef struct WORK_ITEM {

	WORK_QUEUE_ITEM reserved; // work ����
	WORK_CONTEXT context; // work ȣ�� �� �Ѱ����� ������ ( ĳ�����ؼ� ��� ) 
	

}WORK_ITEM, *PWORK_ITEM;

#endif 