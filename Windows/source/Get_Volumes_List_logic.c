#include "Get_Volumes_List.h"

PALL_DEVICE_DRIVES ALL_DEVICE_DRIVES_Start_Node = NULL; // 총 드라이브 연결리스트 시작노드 
PALL_DEVICE_DRIVES ALL_DEVICE_DRIVES_Current_Node = NULL; // 총 드라이브 연결리스트 현재노드 



/*



    연결리스트 부분




*/
PALL_DEVICE_DRIVES Create_ALL_DEVICE_DRIVES_Node(
    PUCHAR Previous_addr,

    PUNICODE_STRING DRIVE_ALPHABET,
    PUNICODE_STRING DRIVE_NT_PATH,

    PUNICODE_STRING USB_Serial,

    PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE
) {
    PALL_DEVICE_DRIVES New_Node = ExAllocatePoolWithTag(NonPagedPool, sizeof(ALL_DEVICE_DRIVES), 'DVDR');
    if (New_Node == NULL) return NULL;
    memset(New_Node, 0, sizeof(ALL_DEVICE_DRIVES));

    if (Previous_addr == NULL) {
        New_Node->Previous_Node = NULL;
    }
    else {
        New_Node->Previous_Node = Previous_addr;
    }



    //DRIVE_ALPHABET
    if (DRIVE_ALPHABET != NULL) {
        New_Node->DRIVE_ALPHABET.Length = DRIVE_ALPHABET->Length;
        New_Node->DRIVE_ALPHABET.MaximumLength = DRIVE_ALPHABET->MaximumLength;
        New_Node->DRIVE_ALPHABET.Buffer = ExAllocatePoolWithTag(NonPagedPool, New_Node->DRIVE_ALPHABET.MaximumLength, 'DVDR');
        if (New_Node->DRIVE_ALPHABET.Buffer == NULL) {
            ExFreePoolWithTag(New_Node, 'DVDR');
            return NULL;
        }
        memcpy(New_Node->DRIVE_ALPHABET.Buffer, DRIVE_ALPHABET->Buffer, New_Node->DRIVE_ALPHABET.MaximumLength);
    }



    //DRIVE_NT_PATH
    if (DRIVE_NT_PATH != NULL) {
        New_Node->DRIVE_NT_PATH.Length = DRIVE_NT_PATH->Length;
        New_Node->DRIVE_NT_PATH.MaximumLength = DRIVE_NT_PATH->MaximumLength;
        New_Node->DRIVE_NT_PATH.Buffer = ExAllocatePoolWithTag(NonPagedPool, New_Node->DRIVE_NT_PATH.MaximumLength, 'DVDR');
        if (New_Node->DRIVE_NT_PATH.Buffer == NULL) {
            ExFreePoolWithTag(New_Node->DRIVE_ALPHABET.Buffer, 'DVDR');
            ExFreePoolWithTag(New_Node, 'DVDR');
            return NULL;
        }
        memcpy(New_Node->DRIVE_NT_PATH.Buffer, DRIVE_NT_PATH->Buffer, New_Node->DRIVE_NT_PATH.MaximumLength);
    }

    //USB_Serial
    if (USB_Serial != NULL) {
        New_Node->USBSTOR_Serial.Length = USB_Serial->Length;
        New_Node->USBSTOR_Serial.MaximumLength = USB_Serial->MaximumLength;
        New_Node->USBSTOR_Serial.Buffer = ExAllocatePoolWithTag(NonPagedPool, New_Node->USBSTOR_Serial.MaximumLength, 'DVDR');
        if (New_Node->USBSTOR_Serial.Buffer == NULL) {
            ExFreePoolWithTag(New_Node->DRIVE_ALPHABET.Buffer, 'DVDR');
            ExFreePoolWithTag(New_Node->DRIVE_NT_PATH.Buffer, 'DVDR');
            ExFreePoolWithTag(New_Node, 'DVDR');
            return NULL;
        }
        memcpy(New_Node->USBSTOR_Serial.Buffer, USB_Serial->Buffer, New_Node->USBSTOR_Serial.MaximumLength);
    }


    // ENUM
    if (DRIVE_DEVICE_TYPE != NULL) {
        New_Node->DRIVE_DEVICE_TYPE = *DRIVE_DEVICE_TYPE;
    }





    New_Node->Next_Node = NULL;

    return New_Node;
}



// APPEND
PALL_DEVICE_DRIVES Append_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES current_Node, PUNICODE_STRING DRIVE_ALPHABET, PUNICODE_STRING DRIVE_NT_PATH, PUNICODE_STRING USB_Serial, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE) {

    PALL_DEVICE_DRIVES New_Node = Create_ALL_DEVICE_DRIVES_Node((PUCHAR)current_Node, DRIVE_ALPHABET, DRIVE_NT_PATH, USB_Serial, DRIVE_DEVICE_TYPE);
    if (New_Node == NULL) return NULL;

    current_Node->Next_Node = (PUCHAR)New_Node;

    New_Node->Previous_Node = (PUCHAR)current_Node;

    return New_Node;
}


// PRINT
VOID Print_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Start_Node) {
    if (Start_Node == NULL) return;

    PALL_DEVICE_DRIVES CURRENT_Node = Start_Node;
    do {

        if (CURRENT_Node->USBSTOR_Serial.Length > 1) {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " DRIVE_ALPHABET => %wZ / DRIVE_NT_PATH => %wZ / USBSTOR_Serial => %wZ / DRIVE_DEVICE_TYPE => %d \n", CURRENT_Node->DRIVE_ALPHABET, CURRENT_Node->DRIVE_NT_PATH, CURRENT_Node->USBSTOR_Serial, CURRENT_Node->DRIVE_DEVICE_TYPE);
        }
        else {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " DRIVE_ALPHABET => %wZ / DRIVE_NT_PATH => %wZ / DRIVE_DEVICE_TYPE => %d \n", CURRENT_Node->DRIVE_ALPHABET, CURRENT_Node->DRIVE_NT_PATH, CURRENT_Node->DRIVE_DEVICE_TYPE);
        }





        CURRENT_Node = (PALL_DEVICE_DRIVES)CURRENT_Node->Next_Node; // 다음 노드 이동
    } while (CURRENT_Node != NULL);


    return;
}




//Finder
PALL_DEVICE_DRIVES Finder_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Start_Node, PUNICODE_STRING IMPORTANT_DRIVE_NT_PATH_hint, PUNICODE_STRING Serial_Number_hint, PDEVICE_DECTECT_ENUM Option_INPUT_DRIVE_DEVICE_TYPE) {
    if (Start_Node == NULL) return NULL;

    PALL_DEVICE_DRIVES CURRENT_Node = Start_Node;
    do {

        if (Option_INPUT_DRIVE_DEVICE_TYPE == NULL && IMPORTANT_DRIVE_NT_PATH_hint != NULL) {
            // NT_PATH
            if (RtlCompareUnicodeString(&CURRENT_Node->DRIVE_NT_PATH, IMPORTANT_DRIVE_NT_PATH_hint, TRUE) == 0) {
                return CURRENT_Node;
            }

        }
        else if (Option_INPUT_DRIVE_DEVICE_TYPE != NULL && IMPORTANT_DRIVE_NT_PATH_hint != NULL) {
            // NT_PATH + ENUM
            if ((RtlCompareUnicodeString(&CURRENT_Node->DRIVE_NT_PATH, IMPORTANT_DRIVE_NT_PATH_hint, TRUE) == 0) && (CURRENT_Node->DRIVE_DEVICE_TYPE == *Option_INPUT_DRIVE_DEVICE_TYPE)) {
                return CURRENT_Node;
            }
        }
        else if (Option_INPUT_DRIVE_DEVICE_TYPE == NULL && IMPORTANT_DRIVE_NT_PATH_hint == NULL && Serial_Number_hint != NULL) {
            // USB 감지용 영역임
            // USB_Serial
            if (RtlCompareUnicodeString(&CURRENT_Node->USBSTOR_Serial, Serial_Number_hint, TRUE) == 0) {
                return CURRENT_Node;
            }
        }


        CURRENT_Node = (PALL_DEVICE_DRIVES)CURRENT_Node->Next_Node; // 다음 노드 이동
    } while (CURRENT_Node != NULL);

    return NULL;
}



//UPDATE
PALL_DEVICE_DRIVES Update_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Spectified_Node_valid, PUNICODE_STRING DRIVE_ALPHABET, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE) {
    if (Spectified_Node_valid == NULL) return NULL;


    if (DRIVE_ALPHABET != NULL) {
        if (

            DRIVE_ALPHABET->Length == 4 && DRIVE_ALPHABET->Buffer[1] == L':' &&
            ((DRIVE_ALPHABET->Buffer[0] >= L'A' && DRIVE_ALPHABET->Buffer[0] <= L'Z') ||
                (DRIVE_ALPHABET->Buffer[0] >= L'a' && DRIVE_ALPHABET->Buffer[0] <= L'z'))

            ) {
            // C: D: 와 같은 문자열인가?

            memset(Spectified_Node_valid->DRIVE_ALPHABET.Buffer, 0, Spectified_Node_valid->DRIVE_ALPHABET.MaximumLength);
            ExFreePoolWithTag(Spectified_Node_valid->DRIVE_ALPHABET.Buffer, 'DVDR');

            Spectified_Node_valid->DRIVE_ALPHABET.Length = DRIVE_ALPHABET->Length;
            Spectified_Node_valid->DRIVE_ALPHABET.MaximumLength = DRIVE_ALPHABET->MaximumLength;
            Spectified_Node_valid->DRIVE_ALPHABET.Buffer = ExAllocatePoolWithTag(NonPagedPool, Spectified_Node_valid->DRIVE_ALPHABET.MaximumLength, 'DVDR');
            if (Spectified_Node_valid->DRIVE_ALPHABET.Buffer == NULL) return NULL;

            memcpy(Spectified_Node_valid->DRIVE_ALPHABET.Buffer, DRIVE_ALPHABET->Buffer, Spectified_Node_valid->DRIVE_ALPHABET.MaximumLength);

        }
    }


    // 필터링 유니코드들
    UNICODE_STRING filter_string[] = {
        {0,},
        {0,}

    };
    RtlInitUnicodeString(&filter_string[0], L"STORAGE#Volume#{"); // 하드디스크
    RtlInitUnicodeString(&filter_string[1], L"STORAGE#Volume#_??_USBSTOR#"); // USB 외장



    // ENUM변경
    if (DRIVE_DEVICE_TYPE != NULL) {
        Spectified_Node_valid->DRIVE_DEVICE_TYPE = *DRIVE_DEVICE_TYPE;
    }




    return Spectified_Node_valid;
}

PALL_DEVICE_DRIVES is_Drives_PATH(PUNICODE_STRING INPUT_ABSOULTE_PATH) {
    if (ALL_DEVICE_DRIVES_Start_Node == NULL || ALL_DEVICE_DRIVES_Current_Node == NULL || INPUT_ABSOULTE_PATH == NULL) return NULL;



    PALL_DEVICE_DRIVES current_NODE = ALL_DEVICE_DRIVES_Start_Node;
    do {

        if (

            current_NODE->DRIVE_ALPHABET.Length == 4 && current_NODE->DRIVE_ALPHABET.Buffer[1] == L':' &&
            ((current_NODE->DRIVE_ALPHABET.Buffer[0] >= L'A' && current_NODE->DRIVE_ALPHABET.Buffer[0] <= L'Z') ||
                (current_NODE->DRIVE_ALPHABET.Buffer[0] >= L'a' && current_NODE->DRIVE_ALPHABET.Buffer[0] <= L'z'))

            ) {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "is_Drives_PATH memcmp(%wZm %wZ) \n", INPUT_ABSOULTE_PATH, current_NODE->DRIVE_NT_PATH);
            if (memcmp(INPUT_ABSOULTE_PATH->Buffer, current_NODE->DRIVE_NT_PATH.Buffer, current_NODE->DRIVE_NT_PATH.Length - 2) == 0) {
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "is_Drives_PATH 알파벳 드라이브 문자 같음 ! %wZ TYPE -> %d \n", current_NODE->DRIVE_ALPHABET, current_NODE->DRIVE_DEVICE_TYPE);
                return current_NODE;
            }

        }



        current_NODE = (PALL_DEVICE_DRIVES)current_NODE->Next_Node;
    } while (current_NODE != NULL);


    return NULL;
}



//특정-노드 삭제 
BOOLEAN Remove_Specified_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES* Start_Node, PALL_DEVICE_DRIVES* Current_Node, PALL_DEVICE_DRIVES Specified_Node) {
    if (Specified_Node == NULL || Start_Node == NULL || Current_Node == NULL || *Start_Node == NULL) return FALSE;

    Specified_Node->Next_Node;
    Specified_Node->Previous_Node;

    if (Specified_Node->Previous_Node == NULL && Specified_Node->Next_Node == NULL) {
        /*
            전역 연결리스트 주소가 처음 주소일 때,
        */
        if (*Start_Node == Specified_Node) {
            *Start_Node = (PALL_DEVICE_DRIVES)Specified_Node->Next_Node;
        }

        if (*Start_Node == *Current_Node) *Current_Node = *Start_Node;


    }
    else if (Specified_Node->Previous_Node == NULL && Specified_Node->Next_Node != NULL) {
        ((PALL_DEVICE_DRIVES)Specified_Node->Next_Node)->Previous_Node = NULL;
        if (*Start_Node == Specified_Node) {
            *Start_Node = (PALL_DEVICE_DRIVES)Specified_Node->Next_Node;
        }

        if (*Start_Node == *Current_Node) *Current_Node = *Start_Node;
    }
    else if (Specified_Node->Previous_Node && Specified_Node->Next_Node) {
        /*
            노드가 중간에 껴있는 경우.
        */
        ((PALL_DEVICE_DRIVES)Specified_Node->Next_Node)->Previous_Node = (PUCHAR)((PALL_DEVICE_DRIVES)Specified_Node->Previous_Node);

    }
    else if (Specified_Node->Previous_Node && Specified_Node->Next_Node == NULL) {
        /*
            노드 마지막인 경우, ( Head아님 )
        */
        ((PALL_DEVICE_DRIVES)Specified_Node->Previous_Node)->Next_Node = NULL;

        if (*Current_Node == Specified_Node) *Current_Node = (PALL_DEVICE_DRIVES)Specified_Node->Previous_Node;


    }
    else {
        return FALSE;
    }

    if (Specified_Node->DRIVE_ALPHABET.Buffer != NULL) {
        ExFreePoolWithTag(Specified_Node->DRIVE_ALPHABET.Buffer, 'DVDR');
    }

    if (Specified_Node->DRIVE_NT_PATH.Buffer != NULL) {
        ExFreePoolWithTag(Specified_Node->DRIVE_NT_PATH.Buffer, 'DVDR');
    }

    if (Specified_Node->USBSTOR_Serial.Buffer != NULL) {
        ExFreePoolWithTag(Specified_Node->USBSTOR_Serial.Buffer, 'DVDR');
    }

    ExFreePoolWithTag(Specified_Node, 'DVDR');

    return TRUE;
}




BOOLEAN Complete_ALL_DEVICE_DRIVED_Node(PALL_DEVICE_DRIVES* Start_Node, PALL_DEVICE_DRIVES* Current_Node) {
    if (*Start_Node == NULL || *Current_Node == NULL) return FALSE;

    PALL_DEVICE_DRIVES current_Node = *Start_Node;

    do {
        PALL_DEVICE_DRIVES tmp = (PALL_DEVICE_DRIVES)current_Node->Next_Node;
        if (

            !(
                current_Node->DRIVE_ALPHABET.Length == 4 && current_Node->DRIVE_ALPHABET.Buffer[1] == L':' &&
                ((current_Node->DRIVE_ALPHABET.Buffer[0] >= L'A' && current_Node->DRIVE_ALPHABET.Buffer[0] <= L'Z') ||
                    (current_Node->DRIVE_ALPHABET.Buffer[0] >= L'a' && current_Node->DRIVE_ALPHABET.Buffer[0] <= L'z'))
                )

            ) {

            // 2차 검증 - USB시리얼이 유효한 경우 삭제하지 않음.  ( 드라이브 문자는 없지만, USB시리얼이 유효하면 해당 노드 삭제 안함 )
            if (current_Node->USBSTOR_Serial.Buffer == NULL || current_Node->USBSTOR_Serial.Length == 0) {

                if (!Remove_Specified_ALL_DEVICE_DRIVES_Node(Start_Node, Current_Node, current_Node)) return FALSE;

            }



        }
        else {
            // current_Node 드라이브 문자가 같지만, 중복이 있을 때는 제외해야함. 

            PALL_DEVICE_DRIVES current_Node_for_drives_char_node = current_Node;
            do {


                current_Node_for_drives_char_node = (PALL_DEVICE_DRIVES)current_Node_for_drives_char_node->Next_Node;
                if (current_Node_for_drives_char_node == NULL) break;


                if (memcmp(current_Node_for_drives_char_node->DRIVE_ALPHABET.Buffer, current_Node->DRIVE_ALPHABET.Buffer, 4) == 0) {
                    if (!Remove_Specified_ALL_DEVICE_DRIVES_Node(Start_Node, Current_Node, current_Node_for_drives_char_node)) return FALSE;
                }

            } while (current_Node_for_drives_char_node != NULL);

        }


        current_Node = tmp;
    } while (current_Node != NULL);



    //Print_ALL_DEVICE_DRIVES_Node(*Start_Node);
    return TRUE;

}