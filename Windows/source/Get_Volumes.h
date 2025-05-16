#ifndef GET_VOLUMES_H
#define GET_VOLUMES_H

#include <ntifs.h>

#include "dlp_enum.h"
#include "Get_Volumes_List.h"

#include "KEVENT_or_KMUTEX_init.h"

#define MAX_DEVICES 256
#pragma warning(disable:4996)


extern POBJECT_TYPE* IoDriverObjectType;
extern K_EVENT_or_MUTEX_struct Get_Volume_KMUTEX;


/*
    연결리스트를 생성하거나 Update될 수 있음

    [+]is_PNP_call 는 PNP 외부 장치 마운트에 의해호출될떄 감지용

    [+]is_remove_mode 는 노드 삭제용인지.

*/
VOID ListMountedDrives(PUNICODE_STRING PNP_Device_Name_from_PNP_UNICODE_STRING, BOOLEAN is_remove_node);


DEVICE_DECTECT_ENUM is_external_Device(PUNICODE_STRING Obj_Dir_NAME, PUNICODE_STRING NT_NAME, PUNICODE_STRING PNP_Device_Name_from_PNP_UNICODE_STRING);// Device인지 확인

BOOLEAN Get_USB_Serial(PUNICODE_STRING INPUT_USB_DIR_INFO, PUNICODE_STRING OUTPUT_USB_SERIAL_NUMBER, ULONG32 Filter_Index, BOOLEAN Filter_with_Ampersand); // PNP를 통해 얻은 USB NT경로에서 시리얼 정보 추출 

// 인수값에 대한 드라이브 문자 존재여부 확인용 / 아웃풋: 드라이브 연결리스트 노드
PALL_DEVICE_DRIVES is_Drives_PATH(PUNICODE_STRING INPUT_ABSOULTE_PATH);


BOOLEAN is_check_NetworkDrive(PUNICODE_STRING NT_DEVICE_PATH); // \Device\Mup \.....\

#endif