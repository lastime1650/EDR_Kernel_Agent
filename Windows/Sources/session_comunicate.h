#ifndef session_comunicate

#include <ntifs.h>

#include "KEVENT_or_KMUTEX_init.h"

#define session_alloc_tag 'EDRs'

VOID Session_Communication(
	K_EVENT_or_MUTEX_struct* locked_Event // "Start_TCP_session.h"에서 잠근 이벤트
);

#endif