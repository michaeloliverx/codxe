#pragma once

#include "platform/platform.h"

#include <cstdint>
#include <xtl.h>

namespace codxe
{

extern "C"
{
    NTSYSAPI VOID NTAPI DbgPrint(IN const char *format, ...);
    NTSYSAPI DWORD NTAPI XamGetCurrentTitleId(VOID);

    NTSYSAPI LONG NTAPI XamGetExecutionId(OUT struct XexExecutionId **execution_id);
}

struct AnsiString
{
    USHORT length;
    USHORT maximum_length;
    PCHAR buffer;
};

struct UnicodeString
{
    USHORT length;
    USHORT maximum_length;
    PWSTR buffer;
};

struct LoaderDataTableEntry
{
    LIST_ENTRY in_load_order_links;
    LIST_ENTRY in_closure_order_links;
    LIST_ENTRY in_initialization_order_links;
    PVOID nt_headers_base;
    PVOID image_base;
    DWORD size_of_nt_image;
    UnicodeString full_dll_name;
    UnicodeString base_dll_name;
    DWORD flags;
    DWORD size_of_full_image;
    PVOID entry_point;
    WORD load_count;
    WORD module_index;
    PVOID dll_base_original;
    DWORD checksum;
    DWORD module_load_flags;
    DWORD time_date_stamp;
    PVOID loaded_imports;
    PVOID xex_header_base;
    union
    {
        AnsiString load_file_name;
        struct
        {
            PVOID closure_root;
            PVOID traversal_parent;
        } as_entry;
    } info;
};

struct XexExecutionId
{
    DWORD media_id;
    DWORD version;
    DWORD base_version;
    union TitleIdValue
    {
        DWORD title_id;
        struct Parts
        {
            WORD publisher_id;
            WORD game_id;
        } parts;
    } title;
    BYTE platform;
    BYTE executable_type;
    BYTE disc_num;
    BYTE discs_in_set;
    DWORD save_game_id;
};

typedef void (*XexpFinishExecutableLoad_t)(LoaderDataTableEntry *module, const char *command_line);

void *ResolveExport(const char *module_name, unsigned int ordinal);
uint32_t GetCurrentTitleId();
uint32_t GetCurrentExecutableTimestamp();
uint32_t GetExecutableTimestamp(const LoaderDataTableEntry *module);
XexpFinishExecutableLoad_t GetXexpFinishExecutableLoad(PlatformKind platform);

} // namespace codxe
