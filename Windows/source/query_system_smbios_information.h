#ifndef query_system_smbios_information_H
#define query_system_smbios_information_H

#include <ntifs.h>
#include <ntddk.h>

#include <wdf.h>
#include <ntstrsafe.h>


#include "API_Loader.h"// ���� API����

//#include "SHA256.h"// �ϵ���� ������ ������� SHA512 ����

#define SMBIOS_TAG 'RSMB'

typedef struct _SMBIOS_TABLE_ENTRY_POINT {
    UCHAR AnchorString[4]; // "_SM_"
    UCHAR EntryPointStructureChecksum;
    UCHAR EntryPointLength;
    UCHAR MajorVersion;
    UCHAR MinorVersion;
    USHORT MaxStructureSize;
    UCHAR EntryPointRevision;
    UCHAR FormattedArea[5];
    UCHAR IntermediateAnchorString[5]; // "_DMI_"
    UCHAR IntermediateChecksum;
    USHORT TableLength;
    ULONG TableAddress;
    USHORT NumberOfStructures;
    UCHAR BCDRevision;
} SMBIOS_TABLE_ENTRY_POINT, * PSMBIOS_TABLE_ENTRY_POINT;

#include "SMBIOS_type_structs.h"// SMBIOS �Ľ̿� ����ü���� ����


NTSTATUS Query_SMBIOS_information(
	PUCHAR* SMBIOS_1_Type_DATA, ULONG32* SMBIOS_1_Type_DATA_SIZE,
	PUCHAR* SMBIOS_2_Type_DATA, ULONG32* SMBIOS_2_Type_DATA_SIZE
);

BOOLEAN Release_Query_SMBIOS_information(
	PUCHAR* SMBIOS_1_Type_DATA,
	PUCHAR* SMBIOS_2_Type_DATA
);

//VOID ParseSMBIOSTable(PVOID tableAddress, USHORT tableLength);



#endif