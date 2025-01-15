#ifndef NETWORK_EVENT

#include <ntifs.h>

/*
	네트워크 핸들러
*/

// 로더
NTSTATUS Network_Event_Loader(PDEVICE_OBJECT input_device);

// 언로더
/*...*/

// 핸들러
#include <wdm.h>
#include <wdf.h>


#define NDIS630
#include <fwpsk.h>


#include <ndis/nblaccessors.h>
#include <fwpmk.h>


#include <ip2string.h>

void FwpsCalloutClassifyFn3(
	_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
	_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
	_Inout_opt_ void* layerData,
	_In_opt_ const void* classifyContext,
	_In_ const FWPS_FILTER3* filter,
	_In_ UINT64 flowContext,
	_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
);


NTSTATUS NotifyFn(
	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	_In_ const GUID* filterKey,
	_Inout_ FWPS_FILTER3* filter
);

void NTAPI FlowDeleteFn(
	_In_ UINT16 layerId,
	_In_ UINT32 calloutId,
	_In_ UINT64 flowContext
);

#endif