#ifndef DLP_ENUM_H
#define DLP_ENUM_H

#include <ntifs.h>
#pragma warning(disable:4996)


// is_external_Device �� ���Ͽ� 
typedef enum DEVICE_DECTECT_ENUM {
    DEVICE_None,
    JUST_VOLUME_DISK,
    Internal_DISK,
    External_DISK_USB,// 3
    External_DISK_ISO,
    External_DISK_CDROM,

    NFS, // ���� ����̺�

    USB_DEVICE_from_PNP // PNP �ݹ��Լ��� �񵿱� �����忡�� ȣ��Ǿ��� ���� �ν��ϴ� �뵵


}DEVICE_DECTECT_ENUM, * PDEVICE_DECTECT_ENUM;




#endif