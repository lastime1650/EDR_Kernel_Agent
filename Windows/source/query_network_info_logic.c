#pragma warning(disable:4996)
#include <ntddk.h>
#include <wdm.h>
#include <netioapi.h> // For GetIfTable2, GetUnicastIpAddressTable, FreeMibTable
#include <ws2def.h>    // For AF_INET
#include <ipifcons.h>  // For IF_TYPE_SOFTWARE_LOOPBACK, IfOperStatusUp
#include <ntstrsafe.h> // For RtlStringCchPrintfA (Optional: for formatting MAC)
#include <ip2string.h>

NTSTATUS Get_Local_Network_info(PUCHAR* MacAddress, ULONG32* output_MacAddress_len,  PCHAR* IPv4Address, ULONG32* output_IPv4Address_len) {
	PMIB_IF_TABLE2 pIfTable = NULL;
	PMIB_UNICASTIPADDRESS_TABLE pIpTable = NULL;
	ULONG i, j;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
	status = GetIfTable2(&pIfTable);
    if (status != STATUS_SUCCESS)
        goto EXIT;

	// 테이블 수 추출
	pIfTable->NumEntries;
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "테이블 -> %d \n", pIfTable->NumEntries);

	//GetMulticastIpAddressTable(AF_INET, &pIpTable);
    status = GetUnicastIpAddressTable(AF_INET, &pIpTable);
    if (status != STATUS_SUCCESS)
        goto EXIT1;

	for (i = 0; i < pIfTable->NumEntries; i++) {
		MIB_IF_ROW2* pIfRow = &pIfTable->Table[i];


		if (pIfRow->OperStatus == IfOperStatusUp &&
			pIfRow->Type != IF_TYPE_SOFTWARE_LOOPBACK &&
			pIfRow->PhysicalAddressLength > 0)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MyNetInfoDriver: 인터페이스 확인 중 - 인덱스 %lu, LUID %llu, 타입 %d, 상태 %d\n",
				pIfRow->InterfaceIndex, pIfRow->InterfaceLuid.Value, pIfRow->Type, pIfRow->OperStatus);

			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MyNetInfoDriver: 후보 인터페이스 찾음 - 인덱스: %lu\n", pIfRow->InterfaceIndex);
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "  MAC 주소: %02X-%02X-%02X-%02X-%02X-%02X\n",
				pIfRow->PhysicalAddress[0], pIfRow->PhysicalAddress[1],
				pIfRow->PhysicalAddress[2], pIfRow->PhysicalAddress[3],
				pIfRow->PhysicalAddress[4], pIfRow->PhysicalAddress[5]);

            // 4. 이 인터페이스와 일치하는 IP 주소를 찾기 위해 IP 주소 반복
            for (j = 0; j < pIpTable->NumEntries; j++) {
                MIB_UNICASTIPADDRESS_ROW* pIpRow = &pIpTable->Table[j];

                // 이 IP가 현재 인터페이스에 속하는지 확인 (인덱스보다 LUID 사용이 더 안정적임)
                // 또한 표준 접두사를 기반으로 루프백/자동 구성 주소가 아닌지 확인 필요 (선택 사항)
                // (이미 AF_INET으로 필터링함)
                // 임시/유효하지 않은 주소를 피하기 위해 SkipAsSource 확인
                if (pIpRow->InterfaceLuid.Value == pIfRow->InterfaceLuid.Value &&
                    pIpRow->SkipAsSource == FALSE)
                {
                    // 주소는 SOCKADDR_INET 공용체(union)에 저장됨
                    if (pIpRow->Address.si_family == AF_INET) {
                        SOCKADDR_IN* pSockAddrIn = (SOCKADDR_IN*)&pIpRow->Address.Ipv4;

                        // 루프백 주소(127.0.0.1)인지 확인
                        // INADDR_LOOPBACK은 네트워크 바이트 순서(x86/x64에서는 little-endian)로 0x0100007F 임
                        if (pSockAddrIn->sin_addr.S_un.S_addr != RtlUlongByteSwap(INADDR_LOOPBACK))
                        {
                            CHAR ipString[16]; // "xxx.xxx.xxx.xxx\0" 용 버퍼

                            if (RtlIpv4AddressToStringA(&pSockAddrIn->sin_addr, ipString) != NULL) {
                                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "  IPv4 주소: %s (PrefixOrigin: %d, SuffixOrigin: %d, DadState: %d)\n",
                                    ipString, pIpRow->PrefixOrigin, pIpRow->SuffixOrigin, pIpRow->DadState);
                                
                                if (MacAddress != NULL) {
                                    UCHAR macAddressString[18]; // "XX-XX-XX-XX-XX-XX\0"
                                    RtlStringCchPrintfA(
                                        (PSTR)macAddressString,       // 대상 버퍼 (PSTR로 캐스팅)
                                        sizeof(macAddressString),    // 대상 버퍼의 크기 (바이트 단위)
                                        "%02X-%02X-%02X-%02X-%02X-%02X", // 서식 문자열
                                        pIfRow->PhysicalAddress[0], pIfRow->PhysicalAddress[1],
                                        pIfRow->PhysicalAddress[2], pIfRow->PhysicalAddress[3],
                                        pIfRow->PhysicalAddress[4], pIfRow->PhysicalAddress[5]);
                                    *MacAddress = ExAllocatePoolWithTag(NonPagedPool, sizeof(macAddressString), 'net');
                                    memset(*MacAddress, 0, sizeof(macAddressString));
                                    RtlCopyMemory(*MacAddress, macAddressString, sizeof(macAddressString));
                                    if (output_MacAddress_len)
                                        *output_MacAddress_len = sizeof(macAddressString);
                                }

                                if (IPv4Address != NULL) {

                                    ULONG32 real_size = 0; 
                                    for (int c = 0; c < sizeof(ipString); c++) {
                                        if (ipString[c] == '\0') {
                                            real_size++;
                                            break;
                                        }

                                        else {
                                            real_size++;
                                        }
                                    }


                                    *IPv4Address = ExAllocatePoolWithTag(NonPagedPool, real_size, 'net');
                                    memset(*IPv4Address, 0, real_size);
                                    RtlCopyMemory(*IPv4Address, ipString, real_size);
                                    if (output_IPv4Address_len)
                                        (*output_IPv4Address_len) = real_size;
                                }
                                    

                                // 첫 번째 유효한 MAC/IPv4 쌍을 찾음
                                status = STATUS_SUCCESS; // 전체 성공 상태
                                goto EXIT2;
                            }
                            else {
                                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MyNetInfoDriver: RtlIpv4AddressToStringA 실패, 상태\n");
                            }
                        }
                        else {
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MyNetInfoDriver: 루프백 IPv4 주소(127.0.0.1) 건너뜀\n");
                        }
                    }
                }
            }
		}
	}

    goto EXIT2;

EXIT2:
    {
        FreeMibTable(pIpTable);
        goto EXIT1;
    }

EXIT1:
    {
        FreeMibTable(pIfTable);
        goto EXIT;
    }
EXIT:
    {

        return status;
    }
}

VOID FREE_Get_LOCAL_network_info(PVOID MacAddress, PVOID Local_Address) {

    if (MacAddress)     
        ExFreePoolWithTag(MacAddress, 'net');
    
    if (Local_Address)
        ExFreePoolWithTag(Local_Address, 'net');
}