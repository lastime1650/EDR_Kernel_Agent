#include "Obregistercallbacks_regtister.h"



// �ݹ� ��� ����ü
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

    // ������
    UNICODE_STRING altitude;
    RtlInitUnicodeString(&altitude, atitude); // ������ altitude ���ڿ� ���

    /*
    g_CallbackContext.Operations.ObjectType = PsProcessType; // ���μ��� �ڵ� ���� ����
    g_CallbackContext.Operations.Operations = OB_OPERATION_HANDLE_CREATE; // �ڵ� ���� �� �ݹ� ȣ��
    g_CallbackContext.Operations.PreOperation = PreOperationCallback; // Prefix �ڵ鷯��!@!@
    g_CallbackContext.Operations.PostOperation = NULL;

    g_CallbackContext.Registration.Version = OB_FLT_REGISTRATION_VERSION;
    g_CallbackContext.Registration.OperationRegistrationCount = 1;
    g_CallbackContext.Registration.Altitude = altitude; // �� ���
    g_CallbackContext.Registration.RegistrationContext = &g_CallbackContext;
    g_CallbackContext.Registration.OperationRegistration = &g_CallbackContext.Operations;
    */
    OB_OPERATION_REGISTRATION operations[] = {
       { PsProcessType, OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE , PreOperationCallback, NULL } // ���μ��� ����͸�
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
#define PROCESS_CREATE_PROCESS      (0x0080)  // ���μ����� �� �ڽ� ���μ����� ����ϴ�.
#define PROCESS_CREATE_THREAD       (0x0002)  // ���μ����� ���ؽ�Ʈ���� �� �����带 ����ϴ�.
#define PROCESS_DUP_HANDLE          (0x0040)  // ����� ��� DuplicateHandle ��ƾ�� ȣ���ϴ� �� ���μ��� ���ؽ�Ʈ���� �ڵ��� �����մϴ�.
#define PROCESS_SET_QUOTA           (0x0100)  // ����� ��� SetProcessWorkingSetSize ��ƾ�� ȣ���ϴ� �� ���μ����� �۾� ���� ũ�⸦ �����մϴ�.
#define PROCESS_SET_INFORMATION     (0x0200)  // ����� ��� SetPriorityClass ��ƾ�� ȣ���ϴ� ���� ���μ��� ������ �����մϴ�.
#define PROCESS_SUSPEND_RESUME      (0x0800)  // ���μ����� �Ͻ� �ߴ��ϰų� �ٽ� �����մϴ�.
#define PROCESS_TERMINATE           (0x0001)  // ����� ��� TerminateProcess ��ƾ�� ȣ���ϴ� ���� ���μ����� �����մϴ�.
#define PROCESS_VM_OPERATION        (0x0008)  // ����� ��� WriteProcessMemory �� VirtualProtectEx ��ƾ�� ȣ���ϴ� �� ���μ����� �ּ� ������ �����մϴ�.
#define PROCESS_VM_WRITE            (0x0020)  // ����� ��� WriteProcessMemory ��ƾ�� ȣ���ϴ� �� ���μ����� �ּ� ������ ���ϴ�.


OB_PREOP_CALLBACK_STATUS PreOperationCallback(
    _In_ PVOID RegistrationContext,
    _Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation
) {
    UNREFERENCED_PARAMETER(RegistrationContext);


    if (OperationInformation->ObjectType == *PsProcessType &&
        OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE) {


        if (Process_Response_Start_Node_Address != NULL) {
            PEPROCESS Process = (PEPROCESS)OperationInformation->Object; // �� �̰ɷ� PID ������ 
            HANDLE PID = PsGetProcessId(Process);

            if (Check_Process_Response(&PID)) {
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " �����ϱ� \n");
                OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;

                // Ȥ�� �𸣴� ����� �ؾ��ϹǷ� ���� ������ �׻� ���� �� ( ���� ���� �ִ� ���� �Ұ����� )
                OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = PROCESS_TERMINATE;

            }

            

        }
    }

    return OB_PREOP_SUCCESS;
}
