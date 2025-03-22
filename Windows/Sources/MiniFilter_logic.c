#include "MiniFilter.h"

PFLT_FILTER gFilterHandle = NULL;

const FLT_OPERATION_REGISTRATION Callback_s[] = {
    /*
    { IRP_MJ_CREATE, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_CREATE_NAMED_PIPE, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_CLOSE, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_READ, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_WRITE, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_QUERY_INFORMATION, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SET_INFORMATION, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_QUERY_EA, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SET_EA, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_FLUSH_BUFFERS, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_QUERY_VOLUME_INFORMATION, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SET_VOLUME_INFORMATION, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_DIRECTORY_CONTROL, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_FILE_SYSTEM_CONTROL, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_DEVICE_CONTROL, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SHUTDOWN, 0, Pre_filter_Handler, NULL }, // No post-operation callback
    { IRP_MJ_LOCK_CONTROL, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_CLEANUP, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_CREATE_MAILSLOT, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_QUERY_SECURITY, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SET_SECURITY, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_POWER, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SYSTEM_CONTROL, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_DEVICE_CHANGE, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_QUERY_QUOTA, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SET_QUOTA, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_PNP, 0, Pre_filter_Handler, NULL },
    */
    { IRP_MJ_CREATE, 0, PRE_filter_Handler, POST_filter_Handler },
{ IRP_MJ_CREATE_NAMED_PIPE, 0, NULL, POST_filter_Handler },
{ IRP_MJ_CLOSE, 0, NULL, POST_filter_Handler },
{ IRP_MJ_READ, 0, NULL, POST_filter_Handler },
{ IRP_MJ_WRITE, 0, NULL, POST_filter_Handler },
{ IRP_MJ_QUERY_INFORMATION, 0, NULL, POST_filter_Handler },
{ IRP_MJ_SET_INFORMATION, 0, NULL, POST_filter_Handler },
{ IRP_MJ_QUERY_EA, 0, NULL, POST_filter_Handler },
{ IRP_MJ_SET_EA, 0, NULL, POST_filter_Handler },
{ IRP_MJ_FLUSH_BUFFERS, 0, NULL, POST_filter_Handler },
{ IRP_MJ_QUERY_VOLUME_INFORMATION, 0, NULL, POST_filter_Handler },
{ IRP_MJ_SET_VOLUME_INFORMATION, 0, NULL, POST_filter_Handler },
{ IRP_MJ_DIRECTORY_CONTROL, 0, NULL, POST_filter_Handler },
{ IRP_MJ_FILE_SYSTEM_CONTROL, 0, NULL, POST_filter_Handler },
{ IRP_MJ_DEVICE_CONTROL, 0, NULL, POST_filter_Handler },
{ IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, NULL, POST_filter_Handler },
{ IRP_MJ_SHUTDOWN, 0, NULL, NULL }, // No post-operation callback
{ IRP_MJ_LOCK_CONTROL, 0, NULL, POST_filter_Handler },
{ IRP_MJ_CLEANUP, 0, NULL, POST_filter_Handler },
{ IRP_MJ_CREATE_MAILSLOT, 0, NULL, POST_filter_Handler },
{ IRP_MJ_QUERY_SECURITY, 0, NULL, POST_filter_Handler },
{ IRP_MJ_SET_SECURITY, 0, NULL, POST_filter_Handler },
{ IRP_MJ_POWER, 0, NULL, POST_filter_Handler },
{ IRP_MJ_SYSTEM_CONTROL, 0, NULL, POST_filter_Handler },
{ IRP_MJ_DEVICE_CHANGE, 0, NULL, POST_filter_Handler },
{ IRP_MJ_QUERY_QUOTA, 0, NULL, POST_filter_Handler },
{ IRP_MJ_SET_QUOTA, 0, NULL, POST_filter_Handler },
{ IRP_MJ_PNP, 0, NULL, POST_filter_Handler },
    { IRP_MJ_OPERATION_END } // Array termination
};

// �� ����̹������� ��� �� ���ʿ�������, �۵��Ϸ��� �ʿ���; 
NTSTATUS InstanceSetupCallback(_In_ PCFLT_RELATED_OBJECTS FltObjects, _In_ FLT_INSTANCE_SETUP_FLAGS Flags, _In_ DEVICE_TYPE VolumeDeviceType, _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType);
VOID InstanceTeardownCallback(_In_ PCFLT_RELATED_OBJECTS FltObjects, _In_ FLT_INSTANCE_TEARDOWN_FLAGS Reason);


NTSTATUS Register_MiniFilter(PDRIVER_OBJECT Input_Driver_obj) {
    if (Input_Driver_obj == NULL) return STATUS_UNSUCCESSFUL;
	NTSTATUS status;

    const FLT_REGISTRATION FilterRegistration = {
        sizeof(FLT_REGISTRATION),       // Size
        FLT_REGISTRATION_VERSION,       // Version
        0,                              // Flags
        NULL,                           // Context
        Callback_s,                      // Operation callbacks // ���� �ڵ鷯�� IRP�� �迭�� ���ε� ����
        NULL,                           // MiniFilterUnload
        InstanceSetupCallback,          // InstanceSetup
        NULL,                           // InstanceQueryTeardown
        InstanceTeardownCallback,       // InstanceTeardownStart
        InstanceTeardownCallback,       // InstanceTeardownComplete
        NULL,                           // GenerateFileName
        NULL,                           // GenerateDestinationFileName
        NULL                            // NormalizeNameComponent
    };

    status = FltRegisterFilter(Input_Driver_obj, &FilterRegistration, &gFilterHandle); // STATUS_OBJECT_NAME_NOT_FOUND �� ��� Minifilter inf���� �� ���� �ý��� ����̹��� Load���� �ʾұ� ����
    if (NT_SUCCESS(status)) {

        share_filter_object = ExAllocatePoolWithTag(NonPagedPool, sizeof(Share_filter_Obj), 'mnob'); 
        memset(share_filter_object, 0, sizeof(Share_filter_Obj));
        KeInitializeSpinLock(&share_filter_object->spinlock);// ���ɶ�
         

        status = FltStartFiltering(gFilterHandle);
        if (!NT_SUCCESS(status)) {
            FltUnregisterFilter(gFilterHandle);
        }
 
    }

	return status;
}

NTSTATUS Unload_Minifilter() {
    NTSTATUS status = STATUS_SUCCESS;

    // 1. ���͸� ����
    if (gFilterHandle != NULL) {
        FltUnregisterFilter(gFilterHandle);
        gFilterHandle = NULL; // �ڵ��� NULL�� �����Ͽ� �� �̻� ������ ������ ǥ��
    }

    // 2. ���� ��ü ����
    if (share_filter_object != NULL) {
        // ���ɶ� ���� (�ʿ��� ���)
        // KeReleaseSpinLock(&share_filter_object->spinlock, ...); // ���� IRQL ���� �ʿ�

        ExFreePoolWithTag(share_filter_object, 'mnob');
        share_filter_object = NULL;
    }

    return status;
}


NTSTATUS
InstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(VolumeDeviceType);
    UNREFERENCED_PARAMETER(VolumeFilesystemType);

    return STATUS_SUCCESS;
}


VOID
InstanceTeardownCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Reason
) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Reason);

}