#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "query_process_information.h"


NTSTATUS PID_to_RealHandle(HANDLE PID, HANDLE* ProcessHandle) {
	PEPROCESS Process;
	NTSTATUS status;

	// 1. ProcessId�� ����Ͽ� EPROCESS ����ü�� �����ɴϴ�.
	status = PsLookupProcessByProcessId(PID, &Process);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	// 2. EPROCESS ����ü�κ��� ���μ��� �ڵ��� ����ϴ�.
	status = ObOpenObjectByPointer(Process, OBJ_KERNEL_HANDLE, NULL, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, ProcessHandle);
	if (!NT_SUCCESS(status)) {
		// EPROCESS ��ü ���� ����
		ObDereferenceObject(Process);
		return status;
	}

	// 3. EPROCESS ��ü ������ �����մϴ�.
	ObDereferenceObject(Process);

	return STATUS_SUCCESS;
}

/*
	Handle ���� �Ķ���ͷ� �����Ͽ� Ư���� ���μ����� ������ ��ȯ�Ѵ�.
*/
//#include "KEVENT_or_KMUTEX_init.h"
//K_EVENT_or_MUTEX_struct kmutex_for_getting_realhandle = { NULL, K_MUTEX, FALSE };

NTSTATUS Query_Process_info(
	HANDLE PID, PROCESSINFOCLASS input_Process_class,
	PUNICODE_STRING* output_unicode

) {
	if (!output_unicode)
		return STATUS_INVALID_PARAMETER;
	
	// Mutex���
	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[BSODcheck��] PID:%llu, irql: %d\n", PID, KeGetCurrentIrql());
	//K_object_init_check_also_lock_ifyouwant(&kmutex_for_getting_realhandle, TRUE);
	
	*output_unicode = NULL;
	NTSTATUS status;

	
		


	if (input_Process_class == ProcessBasicInformation) { // ���μ��� ����
		HANDLE real_process_handle = 0;
		status = PID_to_RealHandle(PID, &real_process_handle);
		if (status != STATUS_SUCCESS) {
			//K_object_lock_Release(&kmutex_for_getting_realhandle);
			return status;
		}

		PROCESS_BASIC_INFORMATION process_basic_info = { 0, };
		ULONG32 process_basic_info_len = 0;
		status = ZwQueryInformationProcess(real_process_handle, ProcessBasicInformation, &process_basic_info, sizeof(process_basic_info), (PULONG)&process_basic_info_len);
		//K_object_lock_Release(&kmutex_for_getting_realhandle);
		return status;
	}
	else if (input_Process_class == ProcessImageFileName) { // ���μ��� ���� ���
		if (output_unicode == NULL) {
			//K_object_lock_Release(&kmutex_for_getting_realhandle);
			return STATUS_UNSUCCESSFUL;
		}

		PEPROCESS eprocess = NULL;
		PsLookupProcessByProcessId(PID, &eprocess);
		if(eprocess==NULL)
			return STATUS_UNSUCCESSFUL;

		SeLocateProcessImageName(eprocess, output_unicode);

		ObDereferenceObject(eprocess);

		return STATUS_SUCCESS;


	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Query_Process_info -> ���� ó���� �� ���� ���μ��� CLASS �Դϴ�.\n");
		//K_object_lock_Release(&kmutex_for_getting_realhandle);
		return STATUS_UNSUCCESSFUL;
	}





}

VOID Query_Process_Image_Name___Release_Free_POOL(PUNICODE_STRING INPUT_Unicode) {
	if (INPUT_Unicode) {
		ExFreePool(INPUT_Unicode);
	}
}



/*
	���� �������� PROCESS ����
*/
#include "File_io.h"
#include "Response_Process.h"
NTSTATUS Query_Running_Processes__(
	PTerminate_Sturct Input_info
) {
	NTSTATUS status;
	ULONG bufferSize = 0;
	PVOID systemInformationBuffer = NULL;
	PSYSTEM_PROCESS_INFORMATION processInfo;

	// 1. �ʿ��� ���� ũ�⸦ ����ϴ�.
	status = ZwQuerySystemInformation(SystemProcessInformation, &bufferSize, 0, &bufferSize);
	if (status != STATUS_INFO_LENGTH_MISMATCH) {
		return status;
	}

	// 2. ���۸� �Ҵ��մϴ�.
	systemInformationBuffer = ExAllocatePoolWithTag(PagedPool, bufferSize, 'Proc');
	if (systemInformationBuffer == NULL) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// 3. ���μ��� ������ �����մϴ�.
	status = ZwQuerySystemInformation(SystemProcessInformation, systemInformationBuffer, bufferSize, NULL);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " 2 ZwQuerySystemInformation - %p \n", status);
	if (!NT_SUCCESS(status)) {
		ExFreePoolWithTag(systemInformationBuffer, 'Proc');
		return status;
	}

	// 4. ���μ��� ������ ��ȸ�մϴ�.
	processInfo = (PSYSTEM_PROCESS_INFORMATION)systemInformationBuffer;
	while (TRUE) {
		// 5. �Է¹��� PID�� ���� ���μ����� PID�� ���մϴ�.
		HANDLE current_pid = processInfo->UniqueProcessId;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " {current_pid} -> %llu \n", current_pid);
		/* Input_info �� ��ȿ�ϸ� '��������' ���డ�� */
		if (Input_info) {
			// 1. ����ũ�� �� ������ ���� Ȯ��.
			ULONG32 got_file_size = 0;
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " {got_file_size} -> %lu \n", got_file_size);
			if (get_file_size(NULL, &current_pid, &got_file_size, TRUE) && ( Input_info->FIle_Size == got_file_size ) ) {
				// 2. SHA256���Ͽ� ���� �õ�. ( ����� )
				// 
				// ���� ���� �䱸�� ��� ����
				if (Input_info->is_needs_terminate) {
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " {is_needs_terminate} \n");
					CHAR current_SHA256[SHA256_String_Byte_Length] = { 0, };
					if (
						FILE_to_INFO(
						NULL,
						&current_pid,
						NULL,
						NULL,
						current_SHA256)
						) 
					{
						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " {processss} - ���� \n");
						if (memcmp(current_SHA256, Input_info->SHA256, (SHA256_String_Byte_Length - 1)) == 0) {
							HANDLE realhandle = 0;
							PID_to_RealHandle(current_pid, &realhandle);
							DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ZwTerminateProcess --> %llu\n", current_pid);

							if (realhandle > 0)
								ZwTerminateProcess(realhandle, STATUS_SUCCESS); // ����
						}
						else {
							DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " {processss} - ���� \n");
						}
						
					}

					
				}
			}
		}
			
			




		// 6. ���� ���μ��� ������ �̵��մϴ�.
		if (processInfo->NextEntryOffset == 0) {
			break;
		}
		processInfo = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)processInfo + processInfo->NextEntryOffset);
	}


	// 8���۸� �����մϴ�.
	ExFreePoolWithTag(systemInformationBuffer, 'Proc');

	return STATUS_SUCCESS;
}