#include "Network_Event.h"

/*
	�̰� ������ GUID�ؼ� ����
*/
#define INITGUID
#include <guiddef.h>

HANDLE EngineHandle = 0;

NTSTATUS GenerateGUID(GUID* output);



NTSTATUS InitializeFilterEngine(
	GUID* provider_key
) {
	if (!provider_key) return STATUS_INVALID_PARAMETER;
	NTSTATUS status;


	// ���� ����
	status = FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, NULL, &EngineHandle);
	if (status != STATUS_SUCCESS) {
		return status;
	}


	// ���� ������� ���� ( Ʈ����� ���� ) 
	status = FwpmTransactionBegin(EngineHandle, 0);
	if (status != STATUS_SUCCESS) {
		return status;
	}




	// GUID ��������
	status = GenerateGUID(provider_key);

	// GUID Ű ����
	FWPM_PROVIDER wfp_provider = { 0, };
	wfp_provider.serviceName = (wchar_t*)L"wfpk";
	wfp_provider.displayData.name = (wchar_t*)L"wfpkm_example_provider";
	wfp_provider.displayData.description = (wchar_t*)L"The provider object for wfp-example";

	wfp_provider.providerKey = *provider_key;

	// ������ ���
	status = FwpmProviderAdd(EngineHandle, &wfp_provider, NULL);
	if ((status) != STATUS_SUCCESS) {
		return status;
	}






	// ���� �����Ϸ�
	status = FwpmTransactionCommit(EngineHandle);
	if ((status) != STATUS_SUCCESS) {
		return status;
	}

	return status;
}


NTSTATUS Set_CallOut(
	PDEVICE_OBJECT DeviceObject,

	GUID* ProviderKey,

	const GUID LayerKey,

	UINT32* WPS_CalloutId,
	UINT32* WPM_CalloutId,
	UINT64* WPM_Filterid
) {
	if (!DeviceObject || !ProviderKey || !WPS_CalloutId || !WPM_CalloutId || !WPM_Filterid) 
		return STATUS_INVALID_PARAMETER;

	NTSTATUS status;

	// ������� ���� ( Ʈ����� ���� ) 
	status = FwpmTransactionBegin(EngineHandle, 0);
	if (status != STATUS_SUCCESS) {
		return status;
	}






	/* ===================���� ���� ����============= */

	/* Fwps */
	// ���� key ���� ( �̴� �ݾƿ� Ű ��ȯ )
	GUID Calloutkey = { 0, };
	status = GenerateGUID(&Calloutkey);

	FWPS_CALLOUT3 callout = { 0, };
	callout.calloutKey = Calloutkey; // Fwpm ��Ͻ� �����Ǵ� Ű��

	callout.flags = 0;
	callout.classifyFn = FwpsCalloutClassifyFn3;
	callout.notifyFn = NotifyFn;
	callout.flowDeleteFn = FlowDeleteFn;

	// Fwps �ݾƿ� ��� ( ��� ��� )
	status = FwpsCalloutRegister3(DeviceObject, &callout, WPS_CalloutId);
	if (status != STATUS_SUCCESS) {
		return status;
	}

	/* Fwpm */
	// wpm�ݾƿ� ��� ( ��å ��� ) 
	FWPM_CALLOUT0 pm_callout = { 0, };
	pm_callout.calloutKey = Calloutkey;
	pm_callout.displayData.name = (wchar_t*)L"wfpkm_example_callout";;
	pm_callout.displayData.description = (wchar_t*)L"The callout object for wfp-example";;
	pm_callout.providerKey = ProviderKey;
	pm_callout.applicableLayer = LayerKey;

	status = FwpmCalloutAdd(EngineHandle, &pm_callout, NULL, WPM_CalloutId);
	if (status != STATUS_SUCCESS) {
		return status;
	}

	// WPM ���� ���
	FWPM_FILTER  fwpm_filter = { 0, };
	//fwpm_filter_id
	fwpm_filter.displayData.name = (wchar_t*)L"wfpkm_example_filter";
	fwpm_filter.displayData.description = (wchar_t*)L"The filter object for wfp-example";
	fwpm_filter.layerKey = LayerKey; // �Ķ���ͷ� ������ ���̾� ����
	fwpm_filter.action.type = FWP_ACTION_CALLOUT_UNKNOWN;
	fwpm_filter.action.calloutKey = Calloutkey;

	// ����ġ�� ������ ���� ���� �����.
	fwpm_filter.weight.type = FWP_UINT64; // ����� �ʵ� ����
	UINT64 weight = 0xffffffffffffffff; // ����ġ ����
	fwpm_filter.weight.uint64 = &weight; // ����ġ ����

	status = FwpmFilterAdd(EngineHandle, &fwpm_filter, NULL, WPM_Filterid);
	if (status != STATUS_SUCCESS) {
		return status;
	}



	// �����Ϸ�
	status = FwpmTransactionCommit(EngineHandle);
	if (status != STATUS_SUCCESS) {
		return status;
	}

	return status;
}















NTSTATUS NDIS_PacketFilter_Register(PDEVICE_OBJECT DeviceObject) {
	if (!DeviceObject) return STATUS_INVALID_PARAMETER;
	NTSTATUS status = STATUS_SUCCESS;

	GUID provider_key = { 0, };
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���� ���� �õ�\n");
	status = InitializeFilterEngine(&provider_key);
	if (status != STATUS_SUCCESS) {
		return status;
	}



	const GUID LayerKey[] = {
		FWPM_LAYER_INBOUND_TRANSPORT_V4, // "Local ����"�� ������ "����"����
		FWPM_LAYER_OUTBOUND_TRANSPORT_V4//, // "Remote �ܺ�"�� ������ "����"����

		// FWPM_LAYER_INBOUND_IPPACKET_V4, // "Local ����"�� ������ "IP"����
		// FWPM_LAYER_OUTBOUND_IPPACKET_V4 // "Remote �ܺ�"�� ������ "IP"����
	};
	for (ULONG32 i = 0; i < sizeof(LayerKey) / sizeof(GUID); i++) {
		UINT32 WPS_CalloutId = 0;
		UINT32 WPM_CalloutId = 0;
		UINT64 WPM_Filterid = 0;

		status = Set_CallOut(
			DeviceObject,
			&provider_key,
			LayerKey[i],

			&WPS_CalloutId,
			&WPM_CalloutId,
			&WPM_Filterid
		);
		if (status != STATUS_SUCCESS) {
			return status;
		}
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "��Ŷ���� ��� �Ϸ�\n");
	return status;
}

NTSTATUS GenerateGUID(GUID* output) {
	if (!output) return STATUS_INVALID_PARAMETER;

	NTSTATUS status;

	do {
		status = ExUuidCreate(output);
		if (status == STATUS_SUCCESS) {
			return STATUS_SUCCESS;
		}
	} while (status == STATUS_RETRY);

	if (status == RPC_NT_UUID_LOCAL_ONLY) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Warning: Local-only UUID generated\n");
		return STATUS_SUCCESS;  // ��� ��������� ��� ����
	}

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to generate UUID, status: %x\n", status);
	return status;
}

// �δ�
NTSTATUS Network_Event_Loader(PDEVICE_OBJECT input_device) {
	return NDIS_PacketFilter_Register(input_device);
}

// ��δ�
/*...*/

// �ڵ鷯
#include <wdm.h>
#include <wdf.h>


#define NDIS630
#include <fwpsk.h>


#include <ndis/nblaccessors.h>
#include <fwpmk.h>


#include <ip2string.h>
BOOLEAN Get_Packet_Size(void* layerData, ULONG32* PacketSize, PUCHAR* Packet_BUffer_Start_Addr) {

	PAGED_CODE();

	ULONG32 packetsize = 0;

	// ��Ŷ������ ����
	if (layerData != NULL) {
		NET_BUFFER_LIST* netBufferList = (NET_BUFFER_LIST*)layerData;
		NET_BUFFER* netBuffer = NET_BUFFER_LIST_FIRST_NB(netBufferList);

		while (netBuffer != NULL) {
			ULONG packetDataLength = NET_BUFFER_DATA_LENGTH(netBuffer);
			//UCHAR* packetData = NdisGetDataBuffer(netBuffer, packetDataLength, NULL, 1, 0);
			//if (packetData != NULL) {
			//	// ��Ŷ �����͸� ó��
			//}
			packetsize += packetDataLength;

			netBuffer = NET_BUFFER_NEXT_NB(netBuffer);
		}
	}

	// ���� ��Ŷ ������ output
	*PacketSize = packetsize;

	if (Packet_BUffer_Start_Addr) {
		PUCHAR Packet_Buffer = ExAllocatePoolWithTag(NonPagedPool, packetsize, 'NET');
		if (!Packet_Buffer) 
			return FALSE;

		//*Packet_BUffer_Start_Addr = Packet_Buffer;


		// ��Ŷ ������ ����
		ULONG32 current_offset = 0;
		if (layerData != NULL) {
			NET_BUFFER_LIST* netBufferList = (NET_BUFFER_LIST*)layerData;
			NET_BUFFER* netBuffer = NET_BUFFER_LIST_FIRST_NB(netBufferList);

			while (netBuffer != NULL) {
				ULONG packetDataLength = NET_BUFFER_DATA_LENGTH(netBuffer);
				UCHAR* packetData = NdisGetDataBuffer(netBuffer, packetDataLength, NULL, 1, 0);
				if (packetData != NULL) {
					//	// ��Ŷ �����͸� ó��
					RtlCopyMemory(Packet_Buffer + current_offset, packetData, packetDataLength);
					current_offset += packetDataLength;
				}

				netBuffer = NET_BUFFER_NEXT_NB(netBuffer);
			}

		}

	}

	return TRUE;
}

// classify �ڵ鷯
#include "DynamicData_Linked_list.h"
#include "Work_Item_struct.h"
#include "DynamicData_2_lengthBuffer.h"
#include "Get_Time.h"
#include "WorkItem_job.h"

#include "Response_Network.h"
BOOLEAN Check_Block_Packet(FWPS_CLASSIFY_OUT0* classifyOut, PCHAR targetRemoteIP_with_null);
void FwpsCalloutClassifyFn3(
	_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
	_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
	_Inout_opt_ void* layerData,
	_In_opt_ const void* classifyContext,
	_In_ const FWPS_FILTER3* filter,
	_In_ UINT64 flowContext,
	_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
) {
	//UNREFERENCED_PARAMETER(inFixedValues);
	UNREFERENCED_PARAMETER(inMetaValues);
	//UNREFERENCED_PARAMETER(layerData);
	UNREFERENCED_PARAMETER(classifyContext);
	UNREFERENCED_PARAMETER(filter);
	UNREFERENCED_PARAMETER(flowContext);

	HANDLE PID = PsGetCurrentProcessId();
	if (PID < (HANDLE)10) {
		return;
	}

	PCHAR Timestamp = Get_Time();

	CHAR LOCAL_IPv4_STR[16] = { 0, }; // ���� IPv4 String
	CHAR REMOTE_IPv4_STR[16] = { 0, }; // ���� IPv4 String

	ULONG32 LOCAL_PORT = 0;
	ULONG32 REMOTE_PORT = 0;

	ULONG32 protocol = 0;
	ULONG32 packetSize = 0;

	//PUCHAR Packet_BUffer = NULL;

	ULONG32 is_INBOUND = 0;


	switch (inFixedValues->layerId) {

	case FWPS_LAYER_INBOUND_TRANSPORT_V4:
	{
		/* TCP / UDP */

		/*
			���� �м�

			1. TCP/UDP Header
			2. Application Data

		*/
		is_INBOUND = 1;

		// ��������
		protocol = inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_PROTOCOL].value.uint8;



		// ��Ʈ
		LOCAL_PORT = (UINT16)inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_PORT].value.uint16; // local port
		REMOTE_PORT = (UINT16)inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_PORT].value.uint16; // remote port
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LOCAL_PORT: %d\n", LOCAL_PORT);



		// IP -> CHAR ���ڿ��� ��ȯ
		IN_ADDR local_ip = { 0, };
		local_ip.S_un.S_addr = RtlUlongByteSwap(inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_ADDRESS].value.uint32);// ���� IP �ּ�
		RtlIpv4AddressToStringA(&local_ip, LOCAL_IPv4_STR);

		IN_ADDR remote_ip = { 0, };
		remote_ip.S_un.S_addr = RtlUlongByteSwap(inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS].value.uint32);// ���� IP �ּ�
		RtlIpv4AddressToStringA(&remote_ip, REMOTE_IPv4_STR);
		if (Check_Block_Packet(classifyOut, REMOTE_IPv4_STR)) // Packet Block üũ
			return;


		// ��Ŷ �ѷ�
		Get_Packet_Size(layerData, &packetSize, NULL);// &Packet_BUffer);

		break;
	}
	case FWPS_LAYER_OUTBOUND_TRANSPORT_V4:
	{
		/* TCP / UDP */

		/*
			���� �м�

			1. TCP/UDP Header
			2. Application Data

		*/
		is_INBOUND = 0;

		// ��������
		protocol = inFixedValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_PROTOCOL].value.uint8;



		// ��Ʈ
		LOCAL_PORT = (UINT16)inFixedValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_PORT].value.uint16; // local port
		REMOTE_PORT = (UINT16)inFixedValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_REMOTE_PORT].value.uint16; // remote port




		// IP -> CHAR ���ڿ��� ��ȯ
		IN_ADDR local_ip = { 0, };
		local_ip.S_un.S_addr = RtlUlongByteSwap(inFixedValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_ADDRESS].value.uint32);// ���� IP �ּ�
		RtlIpv4AddressToStringA(&local_ip, LOCAL_IPv4_STR);

		IN_ADDR remote_ip = { 0, };
		remote_ip.S_un.S_addr = RtlUlongByteSwap(inFixedValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS].value.uint32);// ���� IP �ּ�
		RtlIpv4AddressToStringA(&remote_ip, REMOTE_IPv4_STR);
		if (Check_Block_Packet(classifyOut, REMOTE_IPv4_STR)) // Packet Block üũ
			return;


		// ��Ŷ �ѷ�
		Get_Packet_Size(layerData, &packetSize, NULL);


		break;
	}
	case FWPS_LAYER_INBOUND_IPPACKET_V4:
	{
		IN_ADDR remote_ip = { 0, };
		remote_ip.S_un.S_addr = RtlUlongByteSwap(inFixedValues->incomingValue[FWPS_FIELD_INBOUND_IPPACKET_V4_IP_REMOTE_ADDRESS].value.uint32);// ���� IP �ּ�
		//===========================================================
		// ���� IP �ּҸ� ANSI ���ڿ��� ��ȯ (�� �κ��� �ٽ�!)
		RtlIpv4AddressToStringA(&remote_ip, REMOTE_IPv4_STR);
		if (Check_Block_Packet(classifyOut, REMOTE_IPv4_STR)) // Packet Block üũ
			return;

		return;
	}
	case FWPS_LAYER_OUTBOUND_IPPACKET_V4:
	{
		IN_ADDR remote_ip = { 0, };
		remote_ip.S_un.S_addr = RtlUlongByteSwap(inFixedValues->incomingValue[FWPS_FIELD_OUTBOUND_IPPACKET_V4_IP_REMOTE_ADDRESS].value.uint32);// ���� IP �ּ�
		//===========================================================
		// ���� IP �ּҸ� ANSI ���ڿ��� ��ȯ (�� �κ��� �ٽ�!)
		RtlIpv4AddressToStringA(&remote_ip, REMOTE_IPv4_STR);
		if (Check_Block_Packet(classifyOut, REMOTE_IPv4_STR)) // Packet Block üũ
			return;

		return;
	}
	default:
		return;
	}
	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " LOCAL_IPv4_STR: %s - LOCAL_Port: %d - REMOTE_IPv4_STR: %s - REMOTE_PORT: %d\n", LOCAL_IPv4_STR, LOCAL_PORT, REMOTE_IPv4_STR, REMOTE_PORT);
	PDynamicData Start_Address = NULL;
	PDynamicData Current_Address = NULL;

	

	// ������ ����
	Start_Address = CreateDynamicData( (PUCHAR)&protocol, sizeof(protocol));// protocol_number
	Current_Address = Start_Address;

	Current_Address = AppendDynamicData(Current_Address, (PUCHAR)&is_INBOUND, sizeof(is_INBOUND));// is_Inbound?

	Current_Address = AppendDynamicData(Current_Address, (PUCHAR)&packetSize, sizeof(packetSize));// packetsize
	//Current_Address = AppendDynamicData(Current_Address, (PUCHAR)Packet_BUffer, packetSize);// Real_Packet_Buffer

	Current_Address = AppendDynamicData(Current_Address, (PUCHAR)LOCAL_IPv4_STR, (ULONG32)strlen((PCHAR)LOCAL_IPv4_STR));// Local IP
	Current_Address = AppendDynamicData(Current_Address, (PUCHAR)&LOCAL_PORT, sizeof(LOCAL_PORT));// Local Port

	Current_Address = AppendDynamicData(Current_Address, (PUCHAR)REMOTE_IPv4_STR, (ULONG32)strlen((PCHAR)REMOTE_IPv4_STR)); // Remote IP
	Current_Address = AppendDynamicData(Current_Address, (PUCHAR)&REMOTE_PORT, sizeof(REMOTE_PORT));// Remote Port




	// �����Ҵ� ����
	//if (Packet_BUffer)
		//ExFreePoolWithTag(Packet_BUffer, 'NET');

	// move
	Pmover move = ExAllocatePoolWithTag(NonPagedPool, sizeof(mover), mover_tag);
	if (!move) {
		RemoveALLDynamicData(Start_Address);
		Release_Got_Time(Timestamp);
		return;
	}
	move->cmd = NDIS_Network_Traffic;
	move->PID = PID;
	move->start_node = Start_Address;
	move->timestamp = Timestamp;


	// Work-Item ���
	PWORK_ITEM work = ExAllocatePoolWithTag(NonPagedPool, sizeof(WORK_ITEM), WORK_ITEM_TAG);
	if (!work) {
		RemoveALLDynamicData(Start_Address);
		ExFreePoolWithTag(move, mover_tag);
		Release_Got_Time(Timestamp);
		return;
	}
	work->context.context = move;
	work->context.startroutine = Dyn_2_lenBuff;


	// Work Item�� ť�� �־� ���߿� PASSIVE_LEVEL���� ����
	ExInitializeWorkItem(&work->reserved, WORK_job, work);
	ExQueueWorkItem(&work->reserved, NormalWorkQueue);

	return;
}






/*
	������@@@@@@@@@@@@@@@@@@@@
*/
NTSTATUS NotifyFn(
	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	_In_ const GUID* filterKey,
	_Inout_ FWPS_FILTER3* filter
) {
	UNREFERENCED_PARAMETER(notifyType);
	UNREFERENCED_PARAMETER(filterKey);
	UNREFERENCED_PARAMETER(filter);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "NotifyFn\n");
	return STATUS_SUCCESS;
}

void NTAPI FlowDeleteFn(
	_In_ UINT16 layerId,
	_In_ UINT32 calloutId,
	_In_ UINT64 flowContext
) {
	UNREFERENCED_PARAMETER(layerId);
	UNREFERENCED_PARAMETER(calloutId);
	UNREFERENCED_PARAMETER(flowContext);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FlowDeleteFn\n");
	return;
}


////// [����] /////
BOOLEAN Check_Block_Packet(FWPS_CLASSIFY_OUT0* classifyOut, PCHAR targetRemoteIP_with_null) {
	if (Network_Response_Start_Node_Address) {
		if (is_exist_Network_Response_Data(targetRemoteIP_with_null, (ULONG32)strlen(targetRemoteIP_with_null) + 1)) {
			classifyOut->actionType = FWP_ACTION_BLOCK; // ����
			return TRUE;
		}
		else {
			classifyOut->actionType = FWP_ACTION_PERMIT; // ���
		}
	}
	return FALSE;
}