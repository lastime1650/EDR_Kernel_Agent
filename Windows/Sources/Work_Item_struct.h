#ifndef WORKITEM_STRUCT

#define WORKITEM_STRUCT

#include <ntifs.h>

#define WORK_ITEM_TAG 'WITg'


typedef struct WORK_CONTEXT{

	PVOID context; // 추가적인 매개변수 데이터
	PKSTART_ROUTINE startroutine; // work 안에서 호출할 별도의 함수

}WORK_CONTEXT, * PWORK_CONTEXT;

typedef struct WORK_ITEM {

	WORK_QUEUE_ITEM reserved; // work 전용
	WORK_CONTEXT context; // work 호출 시 넘겨지는 데이터 ( 캐스팅해서 사용 ) 
	

}WORK_ITEM, *PWORK_ITEM;

#endif 