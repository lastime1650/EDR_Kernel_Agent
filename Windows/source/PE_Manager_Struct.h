#ifndef PE_Manager_STRUCT_H
#define PE_Manager_STRUCT_H

#include <ntifs.h>

// DOS Header 시그니처 (MZ)
#define IMAGE_DOS_SIGNATURE           0x5A4D      // "MZ"

// NT Headers 시그니처 (PE)
#define IMAGE_NT_SIGNATURE            0x00004550  // "PE\0\0"

// FILE_HEADER 시그니처
#define IMAGE_FILE_MACHINE_I386       0x014C      // Intel 386 (32-bit)
#define IMAGE_FILE_MACHINE_AMD64      0x8664      // AMD64 (64-bit)

// Optional Header Magic (32-bit or 64-bit)
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10B       // 32-bit
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20B       // 64-bit

// Section Header 시그니처
#define IMAGE_SIZEOF_SHORT_NAME       8           // 섹션 헤더 이름 크기 (8 바이트)

// Data Directories (옵션 헤더의 데이터 디렉토리 수)
#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16       // 데이터 디렉토리 수

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
#define IMAGE_DIRECTORY_ENTRY_RESERVED   15        // Reserved (사용되지 않음)



// Image Characteristics Flags (파일 헤더 특성 플래그)
#define IMAGE_FILE_RELOCS_STRIPPED    0x0001      // Relocation 정보 제거
#define IMAGE_FILE_EXECUTABLE_IMAGE   0x0002      // 실행 가능한 이미지
#define IMAGE_FILE_LINE_NUMS_STRIPPED 0x0004      // 라인 번호 제거
#define IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020     // 2GB 이상의 주소 공간을 사용할 수 있는 이미지

// Subsystem 시그니처
#define IMAGE_SUBSYSTEM_UNKNOWN       0           // 알 수 없는 서브시스템
#define IMAGE_SUBSYSTEM_WINDOWS_GUI   2           // Windows GUI 서브시스템
#define IMAGE_SUBSYSTEM_WINDOWS_CUI   3           // Windows CUI (콘솔) 서브시스템

// DLL Characteristics Flags (옵션 헤더에서 DLL 특성 플래그)
#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE  0x0040 // ASLR 지원
#define IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY 0x0080 // 코드 무결성 검사 강제
#define IMAGE_DLLCHARACTERISTICS_NX_COMPAT     0x0100  // DEP 지원
#define IMAGE_DLLCHARACTERISTICS_NO_ISOLATION  0x0200  // Isolation 사용 안함
#define IMAGE_DLLCHARACTERISTICS_NO_SEH        0x0400  // SEH 사용 안함
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE 0x8000  // 터미널 서버 인식

// Section Characteristics Flags (섹션 헤더에서 섹션 특성 플래그)
#define IMAGE_SCN_CNT_CODE            0x00000020  // 실행 가능한 코드가 포함된 섹션
#define IMAGE_SCN_CNT_INITIALIZED_DATA 0x00000040 // 초기화된 데이터가 포함된 섹션
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080 // 초기화되지 않은 데이터가 포함된 섹션
#define IMAGE_SCN_MEM_EXECUTE         0x20000000  // 실행 가능한 섹션
#define IMAGE_SCN_MEM_READ            0x40000000  // 읽기 가능한 섹션
#define IMAGE_SCN_MEM_WRITE           0x80000000  // 쓰기 가능한 섹션

// Magic Numbers for Optional Header
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10B       // 32-bit PE Optional Header
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20B       // 64-bit PE Optional Header

// 디버그 정보 시그니처
#define IMAGE_DEBUG_TYPE_CODEVIEW     2           // CodeView 디버그 정보
#define IMAGE_DEBUG_TYPE_MISC         4           // 기타 디버그 정보

typedef struct _IMAGE_DOS_HEADER_c {  // DOS .EXE header
    USHORT e_magic;                    // 매직 넘버 (0x5A4D) 'MZ'
    USHORT e_cblp;                     // 파일의 마지막 페이지 크기 (바이트)
    USHORT e_cp;                       // 파일 크기 (512바이트 페이지 단위)
    USHORT e_crlc;                     // 재배치 항목 수
    USHORT e_cparhdr;                  // 헤더 크기 (단위: 16바이트)
    USHORT e_minalloc;                 // 최소 메모리 할당 크기 (단위: 16바이트)
    USHORT e_maxalloc;                 // 최대 메모리 할당 크기 (단위: 16바이트)
    USHORT e_ss;                       // 초기 SS 값
    USHORT e_sp;                       // 초기 SP 값
    USHORT e_csum;                     // 체크섬
    USHORT e_ip;                       // 초기 IP 값
    USHORT e_cs;                       // 초기 CS 값
    USHORT e_lfarlc;                   // 재배치 테이블 파일 오프셋
    USHORT e_ovno;                     // 오버레이 번호
    USHORT e_res[4];                   // 예약 필드
    USHORT e_oemid;                    // OEM 식별자
    USHORT e_oeminfo;                  // OEM 정보
    USHORT e_res2[10];                 // 예약 필드
    ULONG32 e_lfanew;                   // PE 헤더의 파일 오프셋
} IMAGE_DOS_HEADER_c, * PIMAGE_DOS_HEADER_c;




typedef struct _IMAGE_SECTION_HEADER_c {
    UCHAR  Name[8];               // 섹션 이름
    union {
        ULONG PhysicalAddress;
        ULONG VirtualSize;         // 가상 메모리에서의 크기
    } Misc;
    ULONG32  VirtualAddress;         // 가상 주소
    ULONG32  SizeOfRawData;          // 파일에서의 섹션 크기
    ULONG32  PointerToRawData;       // 파일에서의 섹션 시작 위치
    ULONG32  PointerToRelocations;   // 재배치 테이블 포인터
    ULONG32  PointerToLinenumbers;   // 줄 번호 포인터 (사용되지 않음)
    USHORT NumberOfRelocations;    // 재배치 항목 수
    USHORT NumberOfLinenumbers;    // 줄 번호 항목 수 (사용되지 않음)
    ULONG  Characteristics;        // 섹션 특성 (예: 실행 가능, 읽기 가능)
} IMAGE_SECTION_HEADER_c, * PIMAGE_SECTION_HEADER_c;

typedef struct _IMAGE_DATA_DIRECTORY_c {
    ULONG32 VirtualAddress;   // 데이터 디렉터리의 가상 주소 (RVA)
    ULONG32 Size;             // 데이터 디렉터리의 크기
} IMAGE_DATA_DIRECTORY_c, * PIMAGE_DATA_DIRECTORY_c;


typedef struct _IMAGE_FILE_HEADER_c {
    USHORT Machine;               // CPU 아키텍처 (예: x86, x64)
    USHORT NumberOfSections;       // 섹션 개수
    ULONG32  TimeDateStamp;          // 파일 생성 타임스탬프
    ULONG32  PointerToSymbolTable;   // 심볼 테이블 포인터 (사용되지 않음)
    ULONG32  NumberOfSymbols;        // 심볼 수 (사용되지 않음)
    USHORT SizeOfOptionalHeader;   // 선택적 헤더의 크기
    USHORT Characteristics;        // 파일 특성
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
Reserved (사용되지 않음)
*/
typedef struct _IMAGE_OPTIONAL_HEADER64_c {
    USHORT      Magic;                         // 마법 값 (PE32+: 0x20B)
    UCHAR       MajorLinkerVersion;            // 링커 메이저 버전
    UCHAR       MinorLinkerVersion;            // 링커 마이너 버전
    ULONG       SizeOfCode;                    // 코드 섹션의 크기
    ULONG       SizeOfInitializedData;         // 초기화된 데이터 섹션 크기
    ULONG       SizeOfUninitializedData;       // 초기화되지 않은 데이터 섹션 크기
    ULONG       AddressOfEntryPoint;           // 진입점 주소 (RVA)
    ULONG       BaseOfCode;                    // 코드 섹션의 시작 주소 (RVA)
    ULONGLONG   ImageBase;                     // 기본 이미지 로드 주소
    ULONG       SectionAlignment;              // 섹션 정렬 기준
    ULONG       FileAlignment;                 // 파일 정렬 기준
    USHORT      MajorOperatingSystemVersion;   // 운영 체제 메이저 버전
    USHORT      MinorOperatingSystemVersion;   // 운영 체제 마이너 버전
    USHORT      MajorImageVersion;             // 이미지 메이저 버전
    USHORT      MinorImageVersion;             // 이미지 마이너 버전
    USHORT      MajorSubsystemVersion;         // 서브시스템 메이저 버전
    USHORT      MinorSubsystemVersion;         // 서브시스템 마이너 버전
    ULONG       Win32VersionValue;             // Win32 버전 값 (일반적으로 0)
    ULONG       SizeOfImage;                   // 이미지 전체 크기 (섹션 포함)
    ULONG       SizeOfHeaders;                 // 헤더 크기
    ULONG       CheckSum;                      // 체크섬
    USHORT      Subsystem;                     // 서브시스템 (예: Windows GUI, CUI)
    USHORT      DllCharacteristics;            // DLL 특성
    ULONGLONG   SizeOfStackReserve;            // 예약된 스택 크기
    ULONGLONG   SizeOfStackCommit;             // 커밋된 스택 크기
    ULONGLONG   SizeOfHeapReserve;             // 예약된 힙 크기
    ULONGLONG   SizeOfHeapCommit;              // 커밋된 힙 크기
    ULONG       LoaderFlags;                   // 로더 플래그 (일반적으로 0)
    ULONG       NumberOfRvaAndSizes;           // 데이터 디렉터리 항목 수
    IMAGE_DATA_DIRECTORY_c DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];    // 데이터 디렉터리
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
    IMAGE_FILE_HEADER_c FileHeader;           // 파일 헤더
    IMAGE_OPTIONAL_HEADER64_c OptionalHeader; // 선택적 헤더 (64비트 기준)
} IMAGE_NT_HEADERS_c, * PIMAGE_NT_HEADERS_c;


/*
    디지털 인증서 전용
*/
typedef struct WIN_cert {
    ULONG32 Length; //  인증서의 전체 길이
    USHORT Revision; // 인증서 버전
	USHORT CertificateType; // 인증서 타입
	UCHAR Certificate[1]; // 실제 인증서 데이터
}WIN_cert, *PWIN_cert;

#endif