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
    ���Ḯ��Ʈ�� �����ϰų� Update�� �� ����

    [+]is_PNP_call �� PNP �ܺ� ��ġ ����Ʈ�� ����ȣ��ɋ� ������

    [+]is_remove_mode �� ��� ����������.

*/
VOID ListMountedDrives(PUNICODE_STRING PNP_Device_Name_from_PNP_UNICODE_STRING, BOOLEAN is_remove_node);


DEVICE_DECTECT_ENUM is_external_Device(PUNICODE_STRING Obj_Dir_NAME, PUNICODE_STRING NT_NAME, PUNICODE_STRING PNP_Device_Name_from_PNP_UNICODE_STRING);// Device���� Ȯ��

BOOLEAN Get_USB_Serial(PUNICODE_STRING INPUT_USB_DIR_INFO, PUNICODE_STRING OUTPUT_USB_SERIAL_NUMBER, ULONG32 Filter_Index, BOOLEAN Filter_with_Ampersand); // PNP�� ���� ���� USB NT��ο��� �ø��� ���� ���� 

// �μ����� ���� ����̺� ���� ���翩�� Ȯ�ο� / �ƿ�ǲ: ����̺� ���Ḯ��Ʈ ���
PALL_DEVICE_DRIVES is_Drives_PATH(PUNICODE_STRING INPUT_ABSOULTE_PATH);


BOOLEAN is_check_NetworkDrive(PUNICODE_STRING NT_DEVICE_PATH); // \Device\Mup \.....\

#endif