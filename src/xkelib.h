/**
 * Xbox kernal library.
 *
 * Source https://github.com/jogolden/testdev/tree/master/xkelib
 */
#pragma once

#include "xtl.h"

#define EXPORTNUM(x) // Just for documentation, thx XBMC!

// flags used by ExCreateThread
#define EX_CREATE_FLAG_SUSPENDED 0x1    // thread created suspended
#define EX_CREATE_FLAG_SYSTEM 0x2       // create a system thread
#define EX_CREATE_FLAG_TITLE_EXEC 0x100 // title execution thread

typedef long NTSTATUS;

#define PROC_TYPE_SYSTEM 2
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

typedef struct _STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING, *PSTRING;

typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _LDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY InLoadOrderLinks;           // 0x0 sz:0x8
    LIST_ENTRY InClosureOrderLinks;        // 0x8 sz:0x8
    LIST_ENTRY InInitializationOrderLinks; // 0x10 sz:0x8
    PVOID NtHeadersBase;                   // 0x18 sz:0x4
    PVOID ImageBase;                       // 0x1C sz:0x4
    DWORD SizeOfNtImage;                   // 0x20 sz:0x4
    UNICODE_STRING FullDllName;            // 0x24 sz:0x8
    UNICODE_STRING BaseDllName;            // 0x2C sz:0x8
    DWORD Flags;                           // 0x34 sz:0x4
    DWORD SizeOfFullImage;                 // 0x38 sz:0x4
    PVOID EntryPoint;                      // 0x3C sz:0x4
    WORD LoadCount;                        // 0x40 sz:0x2
    WORD ModuleIndex;                      // 0x42 sz:0x2
    PVOID DllBaseOriginal;                 // 0x44 sz:0x4
    DWORD CheckSum;                        // 0x48 sz:0x4
    DWORD ModuleLoadFlags;                 // 0x4C sz:0x4
    DWORD TimeDateStamp;                   // 0x50 sz:0x4
    PVOID LoadedImports;                   // 0x54 sz:0x4
    PVOID XexHeaderBase;                   // 0x58 sz:0x4
    union
    {
        STRING LoadFileName; // 0x5C sz:0x8
        struct
        {
            PVOID ClosureRoot;     // 0x5C sz:0x4 LDR_DATA_TABLE_ENTRY
            PVOID TraversalParent; // 0x60 sz:0x4 LDR_DATA_TABLE_ENTRY
        } asEntry;
    } inf;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY; // size 100
C_ASSERT(sizeof(LDR_DATA_TABLE_ENTRY) == 0x64);

typedef void (*XexpFinishExecutableLoad_t)(PLDR_DATA_TABLE_ENTRY module, const char *commandLine);

typedef struct _XEX_EXECUTION_ID
{
    DWORD MediaID;     // 0x0 sz:0x4
    DWORD Version;     // 0x4 sz:0x4
    DWORD BaseVersion; // 0x8 sz:0x4
    union
    {
        DWORD TitleID; // 0xC sz:0x4
        struct
        {
            WORD PublisherID; // 0xC sz:0x2
            WORD GameID;      // 0xE sz:0x2
        };
    };
    BYTE Platform;                      // 0x10 sz:0x1
    BYTE ExecutableType;                // 0x11 sz:0x1
    BYTE DiscNum;                       // 0x12 sz:0x1
    BYTE DiscsInSet;                    // 0x13 sz:0x1
    DWORD SaveGameID;                   // 0x14 sz:0x4
} XEX_EXECUTION_ID, *PXEX_EXECUTION_ID; // size 24
C_ASSERT(sizeof(XEX_EXECUTION_ID) == 0x18);

typedef struct _EX_TITLE_TERMINATE_REGISTRATION
{
    PVOID NotificationRoutine; // function pointer
    DWORD Priority;            // xam uses 0x7C800000 for early and 0x0 for late
    LIST_ENTRY ListEntry;      // already defined in winnt.h
} EX_TITLE_TERMINATE_REGISTRATION, *PEX_TITLE_TERMINATE_REGISTRATION;
C_ASSERT(sizeof(EX_TITLE_TERMINATE_REGISTRATION) == 0x10);

extern "C"
{
    NTSYSAPI
    EXPORTNUM(3)
    VOID NTAPI DbgPrint(IN const char *s, ...);

    NTSYSAPI
    EXPORTNUM(463)
    DWORD
    NTAPI
    XamGetCurrentTitleId(VOID);

    NTSYSAPI
    EXPORTNUM(13)
    DWORD
    NTAPI
    ExCreateThread(IN PHANDLE pHandle, IN DWORD dwStackSize, IN LPDWORD lpThreadId, IN PVOID apiThreadStartup,
                   IN LPTHREAD_START_ROUTINE lpStartAddress, IN LPVOID lpParameter, IN DWORD dwCreationFlagsMod);

    NTSYSAPI
    EXPORTNUM(21)
    VOID NTAPI ExRegisterTitleTerminateNotification(IN OUT PEX_TITLE_TERMINATE_REGISTRATION pTermStruct,
                                                    IN BOOL bCreate // true create, false destroy existing
    );

    NTSYSAPI
    EXPORTNUM(102)
    DWORD
    NTAPI
    KeGetCurrentProcessType(VOID);

    NTSYSAPI
    EXPORTNUM(300)
    VOID NTAPI RtlInitAnsiString(OUT PSTRING DestinationString, IN const char *SourceString);

    NTSYSAPI
    EXPORTNUM(259)
    NTSTATUS
    NTAPI
    ObCreateSymbolicLink(IN PSTRING SymbolicLinkName, IN PSTRING DeviceName);

    NTSYSAPI
    EXPORTNUM(260)
    NTSTATUS
    NTAPI
    ObDeleteSymbolicLink(IN PSTRING SymbolicLinkName);

    NTSYSAPI
    EXPORTNUM(640)
    NTSTATUS
    NTAPI
    XamGetExecutionId(IN OUT PXEX_EXECUTION_ID *xid);
}
