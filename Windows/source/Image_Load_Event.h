#ifndef IMAGE_LOAD_EVENT

#include <ntifs.h>

/*
	이미지 로드 핸들러
*/

// 로더
NTSTATUS Image_Load_Event_Loader();

// 언로더
NTSTATUS Image_Load_Event_Unloader();

// 핸들러
VOID PLoadImageNotifyRoutine(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo);

#endif