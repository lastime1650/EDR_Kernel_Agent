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

	// ���̺� �� ����
	pIfTable->NumEntries;
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���̺� -> %d \n", pIfTable->NumEntries);

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
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MyNetInfoDriver: �������̽� Ȯ�� �� - �ε��� %lu, LUID %llu, Ÿ�� %d, ���� %d\n",
				pIfRow->InterfaceIndex, pIfRow->InterfaceLuid.Value, pIfRow->Type, pIfRow->OperStatus);

			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MyNetInfoDriver: �ĺ� �������̽� ã�� - �ε���: %lu\n", pIfRow->InterfaceIndex);
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "  MAC �ּ�: %02X-%02X-%02X-%02X-%02X-%02X\n",
				pIfRow->PhysicalAddress[0], pIfRow->PhysicalAddress[1],
				pIfRow->PhysicalAddress[2], pIfRow->PhysicalAddress[3],
				pIfRow->PhysicalAddress[4], pIfRow->PhysicalAddress[5]);

            // 4. �� �������̽��� ��ġ�ϴ� IP �ּҸ� ã�� ���� IP �ּ� �ݺ�
            for (j = 0; j < pIpTable->NumEntries; j++) {
                MIB_UNICASTIPADDRESS_ROW* pIpRow = &pIpTable->Table[j];

                // �� IP�� ���� �������̽��� ���ϴ��� Ȯ�� (�ε������� LUID ����� �� ��������)
                // ���� ǥ�� ���λ縦 ������� ������/�ڵ� ���� �ּҰ� �ƴ��� Ȯ�� �ʿ� (���� ����)
                // (�̹� AF_INET���� ���͸���)
                // �ӽ�/��ȿ���� ���� �ּҸ� ���ϱ� ���� SkipAsSource Ȯ��
                if (pIpRow->InterfaceLuid.Value == pIfRow->InterfaceLuid.Value &&
                    pIpRow->SkipAsSource == FALSE)
                {
                    // �ּҴ� SOCKADDR_INET ����ü(union)�� �����
                    if (pIpRow->Address.si_family == AF_INET) {
                        SOCKADDR_IN* pSockAddrIn = (SOCKADDR_IN*)&pIpRow->Address.Ipv4;

                        // ������ �ּ�(127.0.0.1)���� Ȯ��
                        // INADDR_LOOPBACK�� ��Ʈ��ũ ����Ʈ ����(x86/x64������ little-endian)�� 0x0100007F ��
                        if (pSockAddrIn->sin_addr.S_un.S_addr != RtlUlongByteSwap(INADDR_LOOPBACK))
                        {
                            CHAR ipString[16]; // "xxx.xxx.xxx.xxx\0" �� ����

                            if (RtlIpv4AddressToStringA(&pSockAddrIn->sin_addr, ipString) != NULL) {
                                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "  IPv4 �ּ�: %s (PrefixOrigin: %d, SuffixOrigin: %d, DadState: %d)\n",
                                    ipString, pIpRow->PrefixOrigin, pIpRow->SuffixOrigin, pIpRow->DadState);
                                
                                if (MacAddress != NULL) {
                                    UCHAR macAddressString[18]; // "XX-XX-XX-XX-XX-XX\0"
                                    RtlStringCchPrintfA(
                                        (PSTR)macAddressString,       // ��� ���� (PSTR�� ĳ����)
                                        sizeof(macAddressString),    // ��� ������ ũ�� (����Ʈ ����)
                                        "%02X-%02X-%02X-%02X-%02X-%02X", // ���� ���ڿ�
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
                                    

                                // ù ��° ��ȿ�� MAC/IPv4 ���� ã��
                                status = STATUS_SUCCESS; // ��ü ���� ����
                                goto EXIT2;
                            }
                            else {
                                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MyNetInfoDriver: RtlIpv4AddressToStringA ����, ����\n");
                            }
                        }
                        else {
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MyNetInfoDriver: ������ IPv4 �ּ�(127.0.0.1) �ǳʶ�\n");
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