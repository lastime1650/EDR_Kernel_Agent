#ifndef GET_VOLUMES_LIST_H
#define GET_VOLUMES_LIST_H

#include <ntifs.h>
#pragma warning(disable:4996)

#include "dlp_enum.h"

/*
    ���Ḯ��Ʈ
*/
// �� ����̽� ����̺� ����ϴ� ���Ḯ��Ʈ 
typedef struct ALL_DEVICE_DRIVES {

    PUCHAR Previous_Node;

    UNICODE_STRING DRIVE_ALPHABET;
    UNICODE_STRING DRIVE_NT_PATH; // ���� 
    DEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE; // ��ü ���� enum

    UNICODE_STRING USBSTOR_Serial; // USB�� ���, �̰� ����. 
    UNICODE_STRING USB_PDO_obj; // PNP ����Ʈ�� USB ����̽�. ( �ַ� Node Remove�� )

    PUCHAR Next_Node;

}ALL_DEVICE_DRIVES, * PALL_DEVICE_DRIVES;

extern PALL_DEVICE_DRIVES ALL_DEVICE_DRIVES_Start_Node; // �� ����̺� ���Ḯ��Ʈ ���۳�� 
extern PALL_DEVICE_DRIVES ALL_DEVICE_DRIVES_Current_Node; // �� ����̺� ���Ḯ��Ʈ ������ 

PALL_DEVICE_DRIVES Create_ALL_DEVICE_DRIVES_Node(PUCHAR Previous_addr, PUNICODE_STRING DRIVE_ALPHABET, PUNICODE_STRING DRIVE_NT_PATH, PUNICODE_STRING USB_Serial, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE);// ��� ����
PALL_DEVICE_DRIVES Append_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES current_Node, PUNICODE_STRING DRIVE_ALPHABET, PUNICODE_STRING DRIVE_NT_PATH, PUNICODE_STRING USB_Serial, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE);

//print
VOID Print_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Start_Node);

//finder hint�� ���Ͽ� ��� ���� ������ NULL��ȯ
PALL_DEVICE_DRIVES Finder_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Start_Node, PUNICODE_STRING IMPORTANT_DRIVE_NT_PATH_hint, PUNICODE_STRING Serial_Number_hint, PDEVICE_DECTECT_ENUM Option_INPUT_DRIVE_DEVICE_TYPE);

//������Ʈ
PALL_DEVICE_DRIVES Update_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Spectified_Node_valid, PUNICODE_STRING DRIVE_ALPHABET, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE);// ��� ����

//Ư��-��� ���� 
BOOLEAN Remove_Specified_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES* Start_Node, PALL_DEVICE_DRIVES* Current_Node, PALL_DEVICE_DRIVES Specified_Node);


// ������ �Լ�, �ߺ� �� �ʿ���� ���� ��������
BOOLEAN Complete_ALL_DEVICE_DRIVED_Node(PALL_DEVICE_DRIVES* Start_Node, PALL_DEVICE_DRIVES* Current_Node);



#endif