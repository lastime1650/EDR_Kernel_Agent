#ifndef PE_Manager_STRUCT_H
#define PE_Manager_STRUCT_H

#include <ntifs.h>

// DOS Header �ñ״�ó (MZ)
#define IMAGE_DOS_SIGNATURE           0x5A4D      // "MZ"

// NT Headers �ñ״�ó (PE)
#define IMAGE_NT_SIGNATURE            0x00004550  // "PE\0\0"

// FILE_HEADER �ñ״�ó
#define IMAGE_FILE_MACHINE_I386       0x014C      // Intel 386 (32-bit)
#define IMAGE_FILE_MACHINE_AMD64      0x8664      // AMD64 (64-bit)

// Optional Header Magic (32-bit or 64-bit)
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10B       // 32-bit
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20B       // 64-bit

// Section Header �ñ״�ó
#define IMAGE_SIZEOF_SHORT_NAME       8           // ���� ��� �̸� ũ�� (8 ����Ʈ)

// Data Directories (�ɼ� ����� ������ ���丮 ��)
#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16       // ������ ���丮 ��

// Data Directories index
#define IMAGE_DIRECTORY_ENTRY_EXPORT    0          // Export Table
#define IMAGE_DIRECTORY_ENTRY_IMPORT    1          // Import Table
#define IMAGE_DIRECTORY_ENTRY_RESOURCE  2          // Resource Table
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION 3          // Exception Table
#define IMAGE_DIRECTORY_ENTRY_SECURITY  4          // Certificate Table
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5          // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG     6          // Debug Directory
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE 7       // Architecture Specific Data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR 8          // Global Pointer Table
#define IMAGE_DIRECTORY_ENTRY_TLS       9          // Thread Local Storage (TLS) Table
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10       // Load Configuration Table
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT 11      // Bound Import Table
#define IMAGE_DIRECTORY_ENTRY_IAT        12         // Import Address Table (IAT)
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT 13      // Delay Import Descriptor
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14    // COM Runtime Descriptor (CLI Header)
#define IMAGE_DIRECTORY_ENTRY_RESERVED   15        // Reserved (������ ����)



// Image Characteristics Flags (���� ��� Ư�� �÷���)
#define IMAGE_FILE_RELOCS_STRIPPED    0x0001      // Relocation ���� ����
#define IMAGE_FILE_EXECUTABLE_IMAGE   0x0002      // ���� ������ �̹���
#define IMAGE_FILE_LINE_NUMS_STRIPPED 0x0004      // ���� ��ȣ ����
#define IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020     // 2GB �̻��� �ּ� ������ ����� �� �ִ� �̹���

// Subsystem �ñ״�ó
#define IMAGE_SUBSYSTEM_UNKNOWN       0           // �� �� ���� ����ý���
#define IMAGE_SUBSYSTEM_WINDOWS_GUI   2           // Windows GUI ����ý���
#define IMAGE_SUBSYSTEM_WINDOWS_CUI   3           // Windows CUI (�ܼ�) ����ý���

// DLL Characteristics Flags (�ɼ� ������� DLL Ư�� �÷���)
#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE  0x0040 // ASLR ����
#define IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY 0x0080 // �ڵ� ���Ἲ �˻� ����
#define IMAGE_DLLCHARACTERISTICS_NX_COMPAT     0x0100  // DEP ����
#define IMAGE_DLLCHARACTERISTICS_NO_ISOLATION  0x0200  // Isolation ��� ����
#define IMAGE_DLLCHARACTERISTICS_NO_SEH        0x0400  // SEH ��� ����
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE 0x8000  // �͹̳� ���� �ν�

// Section Characteristics Flags (���� ������� ���� Ư�� �÷���)
#define IMAGE_SCN_CNT_CODE            0x00000020  // ���� ������ �ڵ尡 ���Ե� ����
#define IMAGE_SCN_CNT_INITIALIZED_DATA 0x00000040 // �ʱ�ȭ�� �����Ͱ� ���Ե� ����
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080 // �ʱ�ȭ���� ���� �����Ͱ� ���Ե� ����
#define IMAGE_SCN_MEM_EXECUTE         0x20000000  // ���� ������ ����
#define IMAGE_SCN_MEM_READ            0x40000000  // �б� ������ ����
#define IMAGE_SCN_MEM_WRITE           0x80000000  // ���� ������ ����

// Magic Numbers for Optional Header
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10B       // 32-bit PE Optional Header
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20B       // 64-bit PE Optional Header

// ����� ���� �ñ״�ó
#define IMAGE_DEBUG_TYPE_CODEVIEW     2           // CodeView ����� ����
#define IMAGE_DEBUG_TYPE_MISC         4           // ��Ÿ ����� ����

typedef struct _IMAGE_DOS_HEADER_c {  // DOS .EXE header
    USHORT e_magic;                    // ���� �ѹ� (0x5A4D) 'MZ'
    USHORT e_cblp;                     // ������ ������ ������ ũ�� (����Ʈ)
    USHORT e_cp;                       // ���� ũ�� (512����Ʈ ������ ����)
    USHORT e_crlc;                     // ���ġ �׸� ��
    USHORT e_cparhdr;                  // ��� ũ�� (����: 16����Ʈ)
    USHORT e_minalloc;                 // �ּ� �޸� �Ҵ� ũ�� (����: 16����Ʈ)
    USHORT e_maxalloc;                 // �ִ� �޸� �Ҵ� ũ�� (����: 16����Ʈ)
    USHORT e_ss;                       // �ʱ� SS ��
    USHORT e_sp;                       // �ʱ� SP ��
    USHORT e_csum;                     // üũ��
    USHORT e_ip;                       // �ʱ� IP ��
    USHORT e_cs;                       // �ʱ� CS ��
    USHORT e_lfarlc;                   // ���ġ ���̺� ���� ������
    USHORT e_ovno;                     // �������� ��ȣ
    USHORT e_res[4];                   // ���� �ʵ�
    USHORT e_oemid;                    // OEM �ĺ���
    USHORT e_oeminfo;                  // OEM ����
    USHORT e_res2[10];                 // ���� �ʵ�
    ULONG32 e_lfanew;                   // PE ����� ���� ������
} IMAGE_DOS_HEADER_c, * PIMAGE_DOS_HEADER_c;




typedef struct _IMAGE_SECTION_HEADER_c {
    UCHAR  Name[8];               // ���� �̸�
    union {
        ULONG PhysicalAddress;
        ULONG VirtualSize;         // ���� �޸𸮿����� ũ��
    } Misc;
    ULONG32  VirtualAddress;         // ���� �ּ�
    ULONG32  SizeOfRawData;          // ���Ͽ����� ���� ũ��
    ULONG32  PointerToRawData;       // ���Ͽ����� ���� ���� ��ġ
    ULONG32  PointerToRelocations;   // ���ġ ���̺� ������
    ULONG32  PointerToLinenumbers;   // �� ��ȣ ������ (������ ����)
    USHORT NumberOfRelocations;    // ���ġ �׸� ��
    USHORT NumberOfLinenumbers;    // �� ��ȣ �׸� �� (������ ����)
    ULONG  Characteristics;        // ���� Ư�� (��: ���� ����, �б� ����)
} IMAGE_SECTION_HEADER_c, * PIMAGE_SECTION_HEADER_c;

typedef struct _IMAGE_DATA_DIRECTORY_c {
    ULONG32 VirtualAddress;   // ������ ���͸��� ���� �ּ� (RVA)
    ULONG32 Size;             // ������ ���͸��� ũ��
} IMAGE_DATA_DIRECTORY_c, * PIMAGE_DATA_DIRECTORY_c;


typedef struct _IMAGE_FILE_HEADER_c {
    USHORT Machine;               // CPU ��Ű��ó (��: x86, x64)
    USHORT NumberOfSections;       // ���� ����
    ULONG32  TimeDateStamp;          // ���� ���� Ÿ�ӽ�����
    ULONG32  PointerToSymbolTable;   // �ɺ� ���̺� ������ (������ ����)
    ULONG32  NumberOfSymbols;        // �ɺ� �� (������ ����)
    USHORT SizeOfOptionalHeader;   // ������ ����� ũ��
    USHORT Characteristics;        // ���� Ư��
} IMAGE_FILE_HEADER_c, * PIMAGE_FILE_HEADER_c;

/*
0.Export Table
1.Import Table
2.Resource Table
3.Exception Table
4.Certificate Table
5.Base Relocation Table
6.Debug Directory
Architecture Specific Data
Global Pointer Table
Thread Local Storage (TLS) Table
Load Configuration Table
Bound Import Table
Import Address Table (IAT)
Delay Import Descriptor
COM Runtime Descriptor (CLI Header)
Reserved (������ ����)
*/
typedef struct _IMAGE_OPTIONAL_HEADER64_c {
    USHORT      Magic;                         // ���� �� (PE32+: 0x20B)
    UCHAR       MajorLinkerVersion;            // ��Ŀ ������ ����
    UCHAR       MinorLinkerVersion;            // ��Ŀ ���̳� ����
    ULONG       SizeOfCode;                    // �ڵ� ������ ũ��
    ULONG       SizeOfInitializedData;         // �ʱ�ȭ�� ������ ���� ũ��
    ULONG       SizeOfUninitializedData;       // �ʱ�ȭ���� ���� ������ ���� ũ��
    ULONG       AddressOfEntryPoint;           // ������ �ּ� (RVA)
    ULONG       BaseOfCode;                    // �ڵ� ������ ���� �ּ� (RVA)
    ULONGLONG   ImageBase;                     // �⺻ �̹��� �ε� �ּ�
    ULONG       SectionAlignment;              // ���� ���� ����
    ULONG       FileAlignment;                 // ���� ���� ����
    USHORT      MajorOperatingSystemVersion;   // � ü�� ������ ����
    USHORT      MinorOperatingSystemVersion;   // � ü�� ���̳� ����
    USHORT      MajorImageVersion;             // �̹��� ������ ����
    USHORT      MinorImageVersion;             // �̹��� ���̳� ����
    USHORT      MajorSubsystemVersion;         // ����ý��� ������ ����
    USHORT      MinorSubsystemVersion;         // ����ý��� ���̳� ����
    ULONG       Win32VersionValue;             // Win32 ���� �� (�Ϲ������� 0)
    ULONG       SizeOfImage;                   // �̹��� ��ü ũ�� (���� ����)
    ULONG       SizeOfHeaders;                 // ��� ũ��
    ULONG       CheckSum;                      // üũ��
    USHORT      Subsystem;                     // ����ý��� (��: Windows GUI, CUI)
    USHORT      DllCharacteristics;            // DLL Ư��
    ULONGLONG   SizeOfStackReserve;            // ����� ���� ũ��
    ULONGLONG   SizeOfStackCommit;             // Ŀ�Ե� ���� ũ��
    ULONGLONG   SizeOfHeapReserve;             // ����� �� ũ��
    ULONGLONG   SizeOfHeapCommit;              // Ŀ�Ե� �� ũ��
    ULONG       LoaderFlags;                   // �δ� �÷��� (�Ϲ������� 0)
    ULONG       NumberOfRvaAndSizes;           // ������ ���͸� �׸� ��
    IMAGE_DATA_DIRECTORY_c DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];    // ������ ���͸�
} IMAGE_OPTIONAL_HEADER64_c, * PIMAGE_OPTIONAL_HEADER64_c;
typedef struct _IMAGE_OPTIONAL_HEADER32_c {
    //
    // Standard fields.
    //
    USHORT        Magic;                   // Identifies as PE32 (0x10B)
    UCHAR         MajorLinkerVersion;
    UCHAR         MinorLinkerVersion;
    ULONG         SizeOfCode;
    ULONG         SizeOfInitializedData;
    ULONG         SizeOfUninitializedData;
    ULONG         AddressOfEntryPoint;
    ULONG         BaseOfCode;
    ULONG         BaseOfData;

    //
    // NT additional fields.
    //
    ULONG         ImageBase;               // 32-bit Image Base Address
    ULONG         SectionAlignment;
    ULONG         FileAlignment;
    USHORT        MajorOperatingSystemVersion;
    USHORT        MinorOperatingSystemVersion;
    USHORT        MajorImageVersion;
    USHORT        MinorImageVersion;
    USHORT        MajorSubsystemVersion;
    USHORT        MinorSubsystemVersion;
    ULONG         Win32VersionValue;
    ULONG         SizeOfImage;
    ULONG         SizeOfHeaders;
    ULONG         CheckSum;
    USHORT        Subsystem;
    USHORT        DllCharacteristics;
    ULONG         SizeOfStackReserve;
    ULONG         SizeOfStackCommit;
    ULONG         SizeOfHeapReserve;
    ULONG         SizeOfHeapCommit;
    ULONG         LoaderFlags;
    ULONG         NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY_c DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32_c, * PIMAGE_OPTIONAL_HEADER32_c;


typedef struct _IMAGE_NT_HEADERS_c {
    ULONG32 Signature;                        // PE Signature "PE\0\0" (0x00004550)
    IMAGE_FILE_HEADER_c FileHeader;           // ���� ���
    IMAGE_OPTIONAL_HEADER64_c OptionalHeader; // ������ ��� (64��Ʈ ����)
} IMAGE_NT_HEADERS_c, * PIMAGE_NT_HEADERS_c;


/*
    ������ ������ ����
*/
typedef struct WIN_cert {
    ULONG32 Length; //  �������� ��ü ����
    USHORT Revision; // ������ ����
	USHORT CertificateType; // ������ Ÿ��
	UCHAR Certificate[1]; // ���� ������ ������
}WIN_cert, *PWIN_cert;

#endif