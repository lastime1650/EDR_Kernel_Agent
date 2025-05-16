#ifndef START_TCP_SESSION
#include <ntifs.h>
/*
	서버와 지속적으로 통신 (단일 비동기 스레드)
*/
#include "KEVENT_or_KMUTEX_init.h"



VOID Loop_for_TCP_Session(PVOID Context);



#endif