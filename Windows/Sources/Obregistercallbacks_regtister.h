#ifndef Obregster
#define Obregster

#include "ntifs.h"



NTSTATUS ObregisterCallbacks_Registering(PWCH atitude);


// �ݹ� �Լ� �ڵ鷯
OB_PREOP_CALLBACK_STATUS PreOperationCallback(
    _In_ PVOID RegistrationContext,
    _Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation
);


#endif