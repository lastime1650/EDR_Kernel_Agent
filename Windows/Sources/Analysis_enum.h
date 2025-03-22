#ifndef Analysis_enum
#define Analysis_enum

#include <ntifs.h>


typedef enum Analysis_Topic {
    AGENT = 0,
    SAND_BOX = 1
}Analysis_Topic;

typedef enum Analysis_Command {

    /*
        구조

		(1 xx xx ) : 커널 -> 분석서버
		( 2 xx xx ) : 분석서버 -> 커널

		첫번째 자리 : 명령어의 종류
        두번째 자리 : 종류
        세번째 자리 : 갯수
        
    */

// 커널 -> 분석서버 ( 1 xx xx )
	PsSetCreateProcessNotifyRoutine_Creation = 10101,
	PsSetCreateProcessNotifyRoutine_Remove = 10102,
	PsSetCreateProcessNotifyRoutine_Creation_Detail = 10103,

	PsSetCreateThreadNotifyRoutine_Creation = 10201,
	PsSetCreateThreadNotifyRoutine_Remove = 10202,

	PsSetLoadImageNotifyRoutine_Load = 10301,

	CmRegisterCallbackEx_for_mon = 10401,

	// 네트워크 활동 NDIS
	NDIS_Network_Traffic = 10501,

	// 파일 시스템 활동
	File_System = 10601,
	
	// 차단
	Response_Process = 10710,
	Response_Process_Remove = 10711,

	Response_Network = 10720,
	Response_Network_Remove = 10721,

	Response_File = 10730,
	Response_File_Remove = 10731,
	get_Response_list = 10799, // 현재 차단 등록된 것들 모두 가져오기


// 분석서버 -> 커널 ( 2 xx xx )
    Request_ALL_Monitored_Data = 20101,
    Request_Real_File = 20201,
	Running_Process_list = 20301, // 현재 실행중인 프로세스 목록 추출
	Request_PID_info = 20401, // PID_ to Detail

// 공통 ( 9 xx xx )
	SUCCESS = 90001,
	FAIL = 90002
}Analysis_Command;


#endif