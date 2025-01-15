#include "Registry_Event.h"

LARGE_INTEGER Cookie_for_unload = { 0, };

// 로더
NTSTATUS Registry_Event_Loader(PDRIVER_OBJECT input_driverobj, PWCH altitude_val) {

	UNICODE_STRING altitude;
	RtlInitUnicodeString(&altitude, altitude_val);

	return CmRegisterCallbackEx(
		ExRegistryCallback_for_monitor,
		&altitude,
		input_driverobj,
		NULL,
		&Cookie_for_unload,
		NULL
	);
}

// 언로더
NTSTATUS Registry_Event_Unloader() {
	return CmUnRegisterCallback(
		Cookie_for_unload
	);
}

// 핸들러
#include "is_system_pid.h"

#include "DynamicData_Linked_list.h" // 데이터를 연결리스트로
#include "Analysis_enum.h" // 명령
#include "DynamicData_2_lengthBuffer.h" // 연결리스트 to 버퍼
#include "Get_Time.h" // 당시시간

#include "WorkItem_job.h"
BOOLEAN Registry_EVENT__data_to_node(
	REG_NOTIFY_CLASS KEY_CLASS, PUCHAR KEY_CLASS_INFO,
	PDynamicData* output_startnode
);
NTSTATUS ExRegistryCallback_for_monitor(PVOID CallbackContext, PVOID Argument1, PVOID Argument2) {
	UNREFERENCED_PARAMETER(CallbackContext);

    PDynamicData start_node = NULL;
	HANDLE pid = PsGetCurrentProcessId();
	if (Is_it_System_Process(pid)) {
        return STATUS_SUCCESS;
	}

    // NOTIFY_CLASS로 변환
    REG_NOTIFY_CLASS notify_class = 0;
    RtlCopyMemory(&notify_class, &Argument1, sizeof(notify_class));

	// CMD 추출
	
	// PID 추출
	//HANDLE pid = pid;

	// 동적 데이터 추출
 
	if (!Registry_EVENT__data_to_node(notify_class, Argument2, &start_node)) {
        goto EXIT;
	}

    goto SUCCESS;

FAIL:
	{
		if (start_node)
			RemoveALLDynamicData(start_node);

		goto EXIT;
	}
SUCCESS:
    {
		/*
			전달용 동적데이터
		*/
        PWORK_ITEM work = ExAllocatePoolWithTag(NonPagedPool, sizeof(WORK_ITEM), WORK_ITEM_TAG);
        if (!work) {
            goto FAIL;
        }
        Pmover move = ExAllocatePoolWithTag(NonPagedPool, sizeof(mover), mover_tag);
        if (!move) {
            ExFreePoolWithTag(work, WORK_ITEM_TAG);
            goto FAIL;
        }

        move->cmd = CmRegisterCallbackEx_for_mon;
        move->PID = pid;
        move->start_node = start_node;
        move->timestamp = Get_Time();

        work->context.context = move;
        work->context.startroutine = Dyn_2_lenBuff;

        ExInitializeWorkItem(&work->reserved, WORK_job, work);
        ExQueueWorkItem(&work->reserved, NormalWorkQueue);

		goto EXIT;
	}

EXIT:
	{
		return STATUS_SUCCESS;
	}
   
}




#include "converter_string.h"
NTSTATUS Object_to_NAME(PUCHAR Object, ANSI_STRING* output);
VOID Object_to_NAME_Release(ANSI_STRING* INPUT);

BOOLEAN Registry_EVENT__data_to_node(
	REG_NOTIFY_CLASS KEY_CLASS, PUCHAR KEY_CLASS_INFO,
	PDynamicData* output_startnode
) {
    PDynamicData current = NULL;
    NTSTATUS Status;
    CHAR except_str[] = " ";
	switch (KEY_CLASS) {
    case RegNtPreCreateKeyEx:
    {
        /*
            무엇을 서버에 전달할 것인가?
            1. KEY_CLASS - str
            2. CREATE_KEY - str 
        */
        // 3. Key_Info
        PREG_CREATE_KEY_INFORMATION_V1 pCreateInfo = (PREG_CREATE_KEY_INFORMATION_V1)KEY_CLASS_INFO;

        /*
            1. 해당 KEY_CLASS의 ANSI 전달
        */
        UCHAR keya[] = "RegCreateKeyEx";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        /*
            2. create_key
        */
        if (pCreateInfo->CompleteName->Length > 0) {
            ANSI_STRING create_key = { 0, };
            UNICODE_to_ANSI(&create_key, pCreateInfo->CompleteName);
            current = AppendDynamicData(current, (PUCHAR)create_key.Buffer, create_key.MaximumLength - 1);
            UNICODE_to_ANSI_release(&create_key);
        }
        else {
            current = AppendDynamicData(current, (PUCHAR)except_str, sizeof(except_str) - 1);
        }
        
        
        /*
            3. ACCESS_MASK
        */
        //Current_Address = AppendDynamicData(Current_Address, (PUCHAR) & (ULONG32)pCreateInfo->DesiredAccess, sizeof(ACCESS_MASK), KeGetCurrentIrql());

        break;
    }
    case RegNtPreOpenKeyEx:
    {
        /*
            무엇을 서버에 전달할 것인가?
            1. KEY_CLASS - str
            2. OPEN_KEY - str //
            3. ACCESS_MASK - int // 생성시 권한
        */
        // 3. Key_Info
        PREG_OPEN_KEY_INFORMATION_V1 pOpenInfo = (PREG_OPEN_KEY_INFORMATION_V1)KEY_CLASS_INFO;

        /*
            1. 해당 KEY_CLASS의 ANSI 전달
        */
        UCHAR keya[] = "RegOpenKeyEx";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;
        /*
           2. open_key
       */
        if (pOpenInfo->CompleteName->Length > 0) {
            ANSI_STRING open_key = { 0, };
            UNICODE_to_ANSI(&open_key, pOpenInfo->CompleteName);
            current = AppendDynamicData(current, (PUCHAR)open_key.Buffer, open_key.MaximumLength - 1);
            UNICODE_to_ANSI_release(&open_key);
        }
        else {
            current = AppendDynamicData(current, (PUCHAR)except_str, sizeof(except_str) - 1);
        }
        
        
        /*
            3. ACCESS_MASK
        */
        //Current_Address = AppendDynamicData(Current_Address, (PUCHAR) & (ULONG32)pOpenInfo->DesiredAccess, sizeof(ACCESS_MASK), KeGetCurrentIrql());

        break;
    }
    case RegNtPreDeleteKey:
    {
        // 3. Key_Info
        PREG_DELETE_KEY_INFORMATION pDeleteInfo = (PREG_DELETE_KEY_INFORMATION)KEY_CLASS_INFO;

        /*
            1. 해당 KEY_CLASS의 ANSI 전달
        */
        UCHAR keya[] = "RegDeleteKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        /*
            2. Object name(KEY_CLASS에 따라 다름) 전달
        */
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pDeleteInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength-1);
            
            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreSetValueKey:
    {
        /*
        *  무엇을 서버에 전달할 것인가?
        *
            PVOID           Object; // 설정할 키
              PUNICODE_STRING ValueName; // 값 이름
              ULONG           TitleIndex; // 제목
              ULONG           Type; // DWORD같은 타입?
              PVOID           Data; // 데이터 버퍼(사이즈에 따라 결정)
        */
        // 3. Key_Info
        PREG_SET_VALUE_KEY_INFORMATION pSetValueInfo = (PREG_SET_VALUE_KEY_INFORMATION)KEY_CLASS_INFO;

        /*
            1. 해당 KEY_CLASS의 ANSI 전달
        */
        UCHAR keya[] = "RegSetValueKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        /*
            2. Object name(KEY_CLASS에 따라 다름) 전달
        */
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pSetValueInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreDeleteValueKey:
    {
        /*
        *  무엇을 서버에 전달할 것인가?
        *
            PVOID           Object; // 키
            PUNICODE_STRING ValueName; // 값 이름
        */
        // 3. Key_Info
        PREG_DELETE_VALUE_KEY_INFORMATION pDeleteValueInfo = (PREG_DELETE_VALUE_KEY_INFORMATION)KEY_CLASS_INFO;

        /*
            1. 해당 KEY_CLASS의 ANSI 전달
        */
        UCHAR keya[] = "RegDeleteValueKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        /*
           2. Object name(KEY_CLASS에 따라 다름) 전달
       */
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pDeleteValueInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreRenameKey:
    {
        /*
        * 무엇을 서버에 전달할 것인가 ㅏ
            PVOID           Object; // 키
            PUNICODE_STRING NewName; // 변경 할 값
        */
        // 3. Key_Info
        PREG_RENAME_KEY_INFORMATION pRenameKeyInfo = (PREG_RENAME_KEY_INFORMATION)KEY_CLASS;

        /*
            1. 해당 KEY_CLASS의 ANSI 전달
        */
        UCHAR keya[] = "RegRenameKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        /*
           2. Object name(KEY_CLASS에 따라 다름) 전달
       */
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pRenameKeyInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    /*
    case RegNtPreSetInformationKey:
    {
        
        * 무엇을 서버에 전달할 것인가?
            PVOID                     Object; // 설정하는 키
        
        // 3. Key_Info
    PREG_SET_INFORMATION_KEY_INFORMATION pSetInfoKey = (PREG_SET_INFORMATION_KEY_INFORMATION)KEY_CLASS_INFO;

    
        1. 해당 KEY_CLASS의 ANSI 전달
    
    UCHAR keya[] = "RegSetInformationKey";
    *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
    current = *output_startnode;

    
       2. Object name(KEY_CLASS에 따라 다름) 전달
   
    ANSI_STRING object_name = { 0, };
    Status = Object_to_NAME(pSetInfoKey->Object, &object_name);
    if (Status == STATUS_SUCCESS)
    {
        // 연결 리스트에 ANSI_STRING 추가
        current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

        // ANSI_STRING 메모리 해제
        Object_to_NAME_Release(&object_name);
    }

    break;
    }
    case RegNtPreEnumerateKey:
    {
        // 3. Key_Info
        PREG_ENUMERATE_KEY_INFORMATION pEnumKeyInfo = (PREG_ENUMERATE_KEY_INFORMATION)Argument2;


            1. 해당 KEY_CLASS의 ANSI 전달

        RtlInitAnsiString(&key_class_A, "RegEnumerateKey");
        Current_Address = AppendDynamicData(&Head_Address, Current_Address, key_class_A.Buffer, key_class_A.Length, KeGetCurrentIrql());


            2. Object name(KEY_CLASS에 따라 다름) 전달

        ANSI_STRING AnsiKeyName = { 0, };
        Status = Object_to_NAME(pEnumKeyInfo->Object, &AnsiKeyName);
        if (NT_SUCCESS(Status))
        {
            // 연결 리스트에 ANSI_STRING 추가
            Current_Address = AppendDynamicData(&Head_Address, Current_Address, AnsiKeyName.Buffer, AnsiKeyName.Length, KeGetCurrentIrql());

            // ANSI_STRING 메모리 해제
            RtlFreeAnsiString(&AnsiKeyName);
        }

        break;
    }

    case RegNtPreEnumerateValueKey:
    {
        // 3. Key_Info
        PREG_ENUMERATE_VALUE_KEY_INFORMATION pEnumValueKeyInfo = (PREG_ENUMERATE_VALUE_KEY_INFORMATION)Argument2;


            1. 해당 KEY_CLASS의 ANSI 전달

        RtlInitAnsiString(&key_class_A, "RegEnumerateValueKey");
        Current_Address = AppendDynamicData(&Head_Address, Current_Address, key_class_A.Buffer, key_class_A.Length, KeGetCurrentIrql());


            2. Object name(KEY_CLASS에 따라 다름) 전달

        ANSI_STRING AnsiKeyName = { 0, };
        Status = Object_to_NAME(pEnumValueKeyInfo->Object, &AnsiKeyName);
        if (NT_SUCCESS(Status))
        {
            // 연결 리스트에 ANSI_STRING 추가
            Current_Address = AppendDynamicData(&Head_Address, Current_Address, AnsiKeyName.Buffer, AnsiKeyName.Length, KeGetCurrentIrql());

            // ANSI_STRING 메모리 해제
            RtlFreeAnsiString(&AnsiKeyName);
        }

        break;
    }
   
    case RegNtPreQueryKey:
    {
        // 3. Key_Info
        PREG_QUERY_KEY_INFORMATION pQueryKeyInfo = (PREG_QUERY_KEY_INFORMATION)KEY_CLASS_INFO;

        
            1. 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegQueryKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pQueryKeyInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreQueryValueKey: // 정보 검색
    {
        /*
            1. object
            2. valuename
        
        // 3. Key_Info
        PREG_QUERY_VALUE_KEY_INFORMATION pQueryValueKeyInfo = (PREG_QUERY_VALUE_KEY_INFORMATION)KEY_CLASS_INFO;

        
            1. 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegQueryValueKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pQueryValueKeyInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreQueryMultipleValueKey:
    {
        // 3. Key_Info
        PREG_QUERY_MULTIPLE_VALUE_KEY_INFORMATION pQueryMultiValueKeyInfo = (PREG_QUERY_MULTIPLE_VALUE_KEY_INFORMATION)KEY_CLASS_INFO;

        
            1. 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegQueryMultipleValueKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pQueryMultiValueKeyInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreKeyHandleClose:
    {
        // 3. Key_Info
        PREG_KEY_HANDLE_CLOSE_INFORMATION pCloseInfo = (PREG_KEY_HANDLE_CLOSE_INFORMATION)KEY_CLASS_INFO;

        
            1. 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegKeyHandleClose";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pCloseInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreFlushKey:
    {
        // 3. Key_Info
        // Windows 10 버전 1703 이상
        PREG_FLUSH_KEY_INFORMATION pFlushKeyInfo = (PREG_FLUSH_KEY_INFORMATION)KEY_CLASS_INFO;

        
           . 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegFlushKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pFlushKeyInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreLoadKey:
    {
        // 3. Key_Info
        // Windows 10 버전 1703 이상
        PREG_LOAD_KEY_INFORMATION pLoadKeyInfo = (PREG_LOAD_KEY_INFORMATION)KEY_CLASS_INFO;

        
            1. 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegLoadKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pLoadKeyInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreUnLoadKey:
    {
        // 3. Key_Info
        // Windows 10 버전 1703 이상
        PREG_UNLOAD_KEY_INFORMATION pUnloadKeyInfo = (PREG_UNLOAD_KEY_INFORMATION)KEY_CLASS_INFO;

        
            1. 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegUnLoadKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pUnloadKeyInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreQueryKeySecurity:
    {
        // 3. Key_Info
        // Windows 10 버전 1703 이상
        PREG_QUERY_KEY_SECURITY_INFORMATION pQueryKeySecurityInfo = (PREG_QUERY_KEY_SECURITY_INFORMATION)KEY_CLASS_INFO;

        
            1. 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegQueryKeySecurity";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pQueryKeySecurityInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreSetKeySecurity:
    {
        // 3. Key_Info
        // Windows 10 버전 1703 이상
        PREG_SET_KEY_SECURITY_INFORMATION pSetKeySecurityInfo = (PREG_SET_KEY_SECURITY_INFORMATION)KEY_CLASS_INFO;

        
            1. 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegSetKeySecurity";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pSetKeySecurityInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreRestoreKey:
    {
        // 3. Key_Info
        // Windows 10 버전 1703 이상
        PREG_RESTORE_KEY_INFORMATION pRestoreKeyInfo = (PREG_RESTORE_KEY_INFORMATION)KEY_CLASS_INFO;

        
            1. 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegRestoreKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pRestoreKeyInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreSaveKey:
    {
        // 3. Key_Info
        // Windows 10 버전 1703 이상
        PREG_SAVE_KEY_INFORMATION pSaveKeyInfo = (PREG_SAVE_KEY_INFORMATION)KEY_CLASS_INFO;

        
            1. 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegSaveKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pSaveKeyInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreReplaceKey:
    {
        // 3. Key_Info
        // Windows 10 버전 1703 이상
        PREG_REPLACE_KEY_INFORMATION pReplaceKeyInfo = (PREG_REPLACE_KEY_INFORMATION)KEY_CLASS_INFO;

        
            1. 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegReplaceKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pReplaceKeyInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreQueryKeyName:
    {
        
            1. 오브젝트
            2.
        
        // 3. Key_Info
        // Windows 10 버전 1703 이상
        PREG_QUERY_KEY_NAME pQueryKeyNameInfo = (PREG_QUERY_KEY_NAME)KEY_CLASS_INFO;

        
            1. 해당 KEY_CLASS의 ANSI 전달
        
        UCHAR keya[] = "RegQueryKeyName";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pQueryKeyNameInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    case RegNtPreSaveMergedKey:
    {
        // Windows 10 버전 1703 이상
        PREG_SAVE_MERGED_KEY_INFORMATION pSaveMergedKeyInfo = (PREG_SAVE_MERGED_KEY_INFORMATION)KEY_CLASS_INFO;

        UCHAR keya[] = "RegSaveMergedKey";
        *output_startnode = CreateDynamicData((PUCHAR)keya, sizeof(keya) - 1);
        current = *output_startnode;

        
           2. Object name(KEY_CLASS에 따라 다름) 전달
       
        ANSI_STRING object_name = { 0, };
        Status = Object_to_NAME(pSaveMergedKeyInfo->Object, &object_name);
        if (Status == STATUS_SUCCESS)
        {
            // 연결 리스트에 ANSI_STRING 추가
            current = AppendDynamicData(current, (PUCHAR)object_name.Buffer, object_name.MaximumLength - 1);

            // ANSI_STRING 메모리 해제
            Object_to_NAME_Release(&object_name);
        }

        break;
    }
    */
	default:
	{
		return FALSE;
	}
	}

	return TRUE;
}


NTSTATUS Object_to_NAME(PUCHAR Object, ANSI_STRING* output) {
    if (Object == NULL) {
        UNICODE_STRING test = { 0, };
        RtlInitUnicodeString(&test, L"This Object has Nothing");
        return UNICODE_to_ANSI(output, &test);
    }
    
    POBJECT_NAME_INFORMATION obj_name = NULL;// (POBJECT_NAME_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, 1024, 'DELK');

    ULONG return_size = 0;


    NTSTATUS status = ObQueryNameString(Object, NULL, 0, &return_size);
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "1 ObQueryNameString %p  @ return_size : %d \n", status, return_size);

    // MISMATCH가 아닌 경우는 바로 실패처리
    if (status != STATUS_INFO_LENGTH_MISMATCH) {
        if (status == STATUS_KEY_DELETED) {
            /*
                반환 형식을 일관성있게(string변환) 해야하므로 불가피하게 이런 형태로 구현됨 
            */
            UNICODE_STRING test = { 0, };
            RtlInitUnicodeString(&test, L"This Object has STATUS_KEY_DELETED");
            return UNICODE_to_ANSI(output, &test);
        }
        UNICODE_STRING test = { 0, };
        RtlInitUnicodeString(&test, L"This Object has Nothing");
            return UNICODE_to_ANSI(output, &test);
    }


    obj_name = (POBJECT_NAME_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, return_size, 'DELK');
    if (obj_name) {

        status = ObQueryNameString(Object, obj_name, return_size, &return_size);
        if (status != STATUS_SUCCESS) {
            ExFreePoolWithTag(obj_name, 'DELK');
            return status;
        }

        status = UNICODE_to_ANSI(output, &obj_name->Name);
        ExFreePoolWithTag(obj_name, 'DELK');

        return status;

    }
    else {
        UNICODE_STRING test = { 0, };
        RtlInitUnicodeString(&test, L"This Object has Nothing");
        return UNICODE_to_ANSI(output, &test);
    }

}

VOID Object_to_NAME_Release(ANSI_STRING* INPUT) {
    
    UNICODE_to_ANSI_release(INPUT);

    return;
}