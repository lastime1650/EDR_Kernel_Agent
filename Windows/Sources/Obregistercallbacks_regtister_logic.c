#include "Obregistercallbacks_regtister.h"



// 콜백 등록 구조체
/*
typedef struct _CALLBACK_REGISTRATION_CONTEXT {
    OB_OPERATION_REGISTRATION Operations;
    OB_CALLBACK_REGISTRATION Registration;
    PVOID RegistrationHandle;
} CALLBACK_REGISTRATION_CONTEXT, * PCALLBACK_REGISTRATION_CONTEXT;

CALLBACK_REGISTRATION_CONTEXT g_CallbackContext;
*/
OB_CALLBACK_REGISTRATION g_CallbackRegistration;
PVOID g_CallbackHandle = NULL;

NTSTATUS ObregisterCallbacks_Registering(PWCH atitude) {

    // 고도설정
    UNICODE_STRING altitude;
    RtlInitUnicodeString(&altitude, atitude); // 고유한 altitude 문자열 사용

    /*
    g_CallbackContext.Operations.ObjectType = PsProcessType; // 프로세스 핸들 조작 감시
    g_CallbackContext.Operations.Operations = OB_OPERATION_HANDLE_CREATE; // 핸들 생성 시 콜백 호출
    g_CallbackContext.Operations.PreOperation = PreOperationCallback; // Prefix 핸들러만!@!@
    g_CallbackContext.Operations.PostOperation = NULL;

    g_CallbackContext.Registration.Version = OB_FLT_REGISTRATION_VERSION;
    g_CallbackContext.Registration.OperationRegistrationCount = 1;
    g_CallbackContext.Registration.Altitude = altitude; // 고도 등록
    g_CallbackContext.Registration.RegistrationContext = &g_CallbackContext;
    g_CallbackContext.Registration.OperationRegistration = &g_CallbackContext.Operations;
    */
    OB_OPERATION_REGISTRATION operations[] = {
       { PsProcessType, OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE , PreOperationCallback, NULL } // 프로세스 모니터링
    };

    g_CallbackRegistration = (OB_CALLBACK_REGISTRATION){
        .Version = OB_FLT_REGISTRATION_VERSION,
        .OperationRegistrationCount = ARRAYSIZE(operations),
        .Altitude = altitude,
        .RegistrationContext = NULL,
        .OperationRegistration = operations
    };

    //status = ObRegisterCallbacks(&g_CallbackContext.Registration, &g_CallbackContext.RegistrationHandle);

    return ObRegisterCallbacks(&g_CallbackRegistration, &g_CallbackHandle);

}

#include "Response_Process.h"
#include "File_io.h"
#define PROCESS_CREATE_PROCESS      (0x0080)  // 프로세스의 새 자식 프로세스를 만듭니다.
#define PROCESS_CREATE_THREAD       (0x0002)  // 프로세스의 컨텍스트에서 새 스레드를 만듭니다.
#define PROCESS_DUP_HANDLE          (0x0040)  // 사용자 모드 DuplicateHandle 루틴을 호출하는 등 프로세스 컨텍스트에서 핸들을 복제합니다.
#define PROCESS_SET_QUOTA           (0x0100)  // 사용자 모드 SetProcessWorkingSetSize 루틴을 호출하는 등 프로세스의 작업 집합 크기를 설정합니다.
#define PROCESS_SET_INFORMATION     (0x0200)  // 사용자 모드 SetPriorityClass 루틴을 호출하는 등의 프로세스 설정을 수정합니다.
#define PROCESS_SUSPEND_RESUME      (0x0800)  // 프로세스를 일시 중단하거나 다시 시작합니다.
#define PROCESS_TERMINATE           (0x0001)  // 사용자 모드 TerminateProcess 루틴을 호출하는 등의 프로세스를 종료합니다.
#define PROCESS_VM_OPERATION        (0x0008)  // 사용자 모드 WriteProcessMemory 및 VirtualProtectEx 루틴을 호출하는 등 프로세스의 주소 공간을 수정합니다.
#define PROCESS_VM_WRITE            (0x0020)  // 사용자 모드 WriteProcessMemory 루틴을 호출하는 등 프로세스의 주소 공간에 씁니다.


OB_PREOP_CALLBACK_STATUS PreOperationCallback(
    _In_ PVOID RegistrationContext,
    _Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation
) {
    UNREFERENCED_PARAMETER(RegistrationContext);


    if (OperationInformation->ObjectType == *PsProcessType &&
        OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE) {


        if (Process_Response_Start_Node_Address != NULL) {
            PEPROCESS Process = (PEPROCESS)OperationInformation->Object; // 꼭 이걸로 PID 얻어야함 
            HANDLE PID = PsGetProcessId(Process);

            if (Check_Process_Response(&PID)) {
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " 차단하기 \n");
                OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;

                // 혹시 모르니 종료는 해야하므로 종료 권한은 항상 주자 ㅋ ( 권한 새로 주는 것은 불가능함 )
                OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = PROCESS_TERMINATE;

            }

            

        }
    }

    return OB_PREOP_SUCCESS;
}
