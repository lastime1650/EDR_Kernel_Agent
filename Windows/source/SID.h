#ifndef SIDs_H
#define SIDs_H

#include <ntifs.h>


typedef enum Users_for_a_SID {
	SYSTEM,
	NETWORK_SERVICE,
	LOCAL_SERVICE
}Users_for_a_SID;

NTSTATUS Get_User_SID_with_Alloc(PSID* out_sid, Users_for_a_SID choice_the_user);

VOID SID_Release(PSID* in_sid);

#endif