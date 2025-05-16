#ifndef DLP_ENUM_H
#define DLP_ENUM_H

#include <ntifs.h>
#pragma warning(disable:4996)


// is_external_Device 의 리턴용 
typedef enum DEVICE_DECTECT_ENUM {
    DEVICE_None,
    JUST_VOLUME_DISK,
    Internal_DISK,
    External_DISK_USB,// 3
    External_DISK_ISO,
    External_DISK_CDROM,

    NFS, // 공유 드라이브

    USB_DEVICE_from_PNP // PNP 콜백함수의 비동기 스레드에서 호출되었을 때를 인식하는 용도


}DEVICE_DECTECT_ENUM, * PDEVICE_DECTECT_ENUM;




#endif