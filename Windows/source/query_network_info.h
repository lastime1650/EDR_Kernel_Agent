#ifndef QUERY_NET_INFO

#include <ntddk.h>
#include <wdm.h>
#include <netioapi.h> // For GetIfTable2, GetUnicastIpAddressTable, FreeMibTable
#include <ws2def.h>    // For AF_INET
#include <ipifcons.h>  // For IF_TYPE_SOFTWARE_LOOPBACK, IfOperStatusUp
#include <ntstrsafe.h> // For RtlStringCchPrintfA (Optional: for formatting MAC)
#include <ip2string.h>

NTSTATUS Get_Local_Network_info(PUCHAR* MacAddress, ULONG32* output_MacAddress_len, PCHAR* IPv4Address, ULONG32* output_IPv4Address_len);

VOID FREE_Get_LOCAL_network_info(PVOID MacAddress, PVOID Local_Address);

#endif