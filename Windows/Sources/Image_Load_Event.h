#ifndef IMAGE_LOAD_EVENT

#include <ntifs.h>

/*
	�̹��� �ε� �ڵ鷯
*/

// �δ�
NTSTATUS Image_Load_Event_Loader();

// ��δ�
NTSTATUS Image_Load_Event_Unloader();

// �ڵ鷯
VOID PLoadImageNotifyRoutine(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo);

#endif