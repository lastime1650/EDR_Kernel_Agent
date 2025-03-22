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

// 이 드라이버에서는 사실 상 불필요하지만, 작동하려면 필요함; 
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
        Callback_s,                      // Operation callbacks // 실제 핸들러와 IRP가 배열로 매핑된 정보
        NULL,                           // MiniFilterUnload
        InstanceSetupCallback,          // InstanceSetup
        NULL,                           // InstanceQueryTeardown
        InstanceTeardownCallback,       // InstanceTeardownStart
        InstanceTeardownCallback,       // InstanceTeardownComplete
        NULL,                           // GenerateFileName
        NULL,                           // GenerateDestinationFileName
        NULL                            // NormalizeNameComponent
    };

    status = FltRegisterFilter(Input_Driver_obj, &FilterRegistration, &gFilterHandle); // STATUS_OBJECT_NAME_NOT_FOUND 인 경우 Minifilter inf파일 및 파일 시스템 드라이버로 Load하지 않았기 때문
    if (NT_SUCCESS(status)) {

        share_filter_object = ExAllocatePoolWithTag(NonPagedPool, sizeof(Share_filter_Obj), 'mnob'); 
        memset(share_filter_object, 0, sizeof(Share_filter_Obj));
        KeInitializeSpinLock(&share_filter_object->spinlock);// 스핀락
         

        status = FltStartFiltering(gFilterHandle);
        if (!NT_SUCCESS(status)) {
            FltUnregisterFilter(gFilterHandle);
        }
 
    }

	return status;
}

NTSTATUS Unload_Minifilter() {
    NTSTATUS status = STATUS_SUCCESS;

    // 1. 필터링 중지
    if (gFilterHandle != NULL) {
        FltUnregisterFilter(gFilterHandle);
        gFilterHandle = NULL; // 핸들을 NULL로 설정하여 더 이상 사용되지 않음을 표시
    }

    // 2. 공유 객체 해제
    if (share_filter_object != NULL) {
        // 스핀락 해제 (필요한 경우)
        // KeReleaseSpinLock(&share_filter_object->spinlock, ...); // 이전 IRQL 저장 필요

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