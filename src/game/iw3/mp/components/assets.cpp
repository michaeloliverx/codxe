#include "pch.h"
#include "assets.h"

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)
#endif

#ifndef INVALID_FILE_SIZE
#define INVALID_FILE_SIZE ((DWORD) - 1)
#endif

namespace iw3
{
namespace mp
{

namespace
{
Detour DB_LinkXAssetEntry_Detour;

const size_t MAX_OVERRIDE_CACHE_ENTRIES = 512;
const size_t STRINGTABLE_VALUE_ALIGNMENT = sizeof(const char *);

enum OverrideCacheState
{
    CACHE_EMPTY,
    CACHE_LOADED
};

struct OverrideCacheEntry
{
    OverrideCacheState state;
    XAssetType type;
    char name[MAX_PATH];
    void *storage;
    DWORD storage_size;
    DWORD payload_size;
    StringTable stringtable;
};

OverrideCacheEntry *s_override_cache = nullptr;
size_t s_override_cache_capacity = 0;

const char *ProcessTypeName(DWORD process_type)
{
    return process_type == PROC_TYPE_SYSTEM ? "system" : "title";
}

size_t AlignSize(size_t value, size_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

void *AllocVirtualBlock(size_t size, const char *reason)
{
    const DWORD process_type = KeGetCurrentProcessType();
    DbgPrint("[codxe][assets] VirtualAlloc begin. reason='%s' size=%u processType=%u(%s)\n",
             reason ? reason : "<null>", static_cast<unsigned int>(size), process_type, ProcessTypeName(process_type));

    void *storage = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (storage == nullptr)
    {
        const DWORD error = GetLastError();
        DbgPrint("[codxe][assets] VirtualAlloc failed. reason='%s' size=%u processType=%u(%s) error=0x%08X\n",
                 reason ? reason : "<null>", static_cast<unsigned int>(size), process_type,
                 ProcessTypeName(process_type), error);
    }
    else
    {
        DbgPrint("[codxe][assets] VirtualAlloc end. reason='%s' ptr=%p size=%u processType=%u(%s)\n",
                 reason ? reason : "<null>", storage, static_cast<unsigned int>(size), process_type,
                 ProcessTypeName(process_type));
    }

    return storage;
}

bool FreeVirtualBlock(void *storage, DWORD size, const char *reason)
{
    if (storage == nullptr)
    {
        return true;
    }

    const DWORD process_type = KeGetCurrentProcessType();
    DbgPrint("[codxe][assets] VirtualFree begin. reason='%s' ptr=%p size=%u processType=%u(%s)\n",
             reason ? reason : "<null>", storage, size, process_type, ProcessTypeName(process_type));

    const BOOL result = VirtualFree(storage, 0, MEM_RELEASE);
    if (!result)
    {
        const DWORD error = GetLastError();
        DbgPrint("[codxe][assets] VirtualFree failed. reason='%s' ptr=%p size=%u processType=%u(%s) error=0x%08X\n",
                 reason ? reason : "<null>", storage, size, process_type, ProcessTypeName(process_type), error);
        return false;
    }

    DbgPrint("[codxe][assets] VirtualFree end. reason='%s' ptr=%p size=%u processType=%u(%s)\n",
             reason ? reason : "<null>", storage, size, process_type, ProcessTypeName(process_type));
    return true;
}

bool ResetCacheEntry(OverrideCacheEntry &entry, DWORD &storage_bytes)
{
    bool freed = false;
    if (entry.storage != nullptr)
    {
        if (FreeVirtualBlock(entry.storage, entry.storage_size, entry.name))
        {
            storage_bytes += entry.storage_size;
            freed = true;
        }
    }

    memset(&entry, 0, sizeof(entry));
    return freed;
}

void ClearOverrideCache()
{
    DbgPrint("[codxe][assets] ClearOverrideCache begin. cache=%p capacity=%u\n", s_override_cache,
             static_cast<unsigned int>(s_override_cache_capacity));

    if (s_override_cache == nullptr)
    {
        DbgPrint("[codxe][assets] ClearOverrideCache end. no cache allocated.\n");
        return;
    }

    unsigned int loaded_entries = 0;
    unsigned int free_attempts = 0;
    unsigned int free_successes = 0;
    unsigned int free_failures = 0;
    DWORD freed_storage_bytes = 0;
    for (size_t i = 0; i < s_override_cache_capacity; ++i)
    {
        if (s_override_cache[i].state == CACHE_LOADED)
        {
            ++loaded_entries;
        }

        if (s_override_cache[i].storage != nullptr)
        {
            ++free_attempts;
        }

        if (ResetCacheEntry(s_override_cache[i], freed_storage_bytes))
        {
            ++free_successes;
        }
        else if (free_attempts != free_successes + free_failures)
        {
            ++free_failures;
        }
    }

    DbgPrint("[codxe][assets] ClearOverrideCache end. loaded=%u freeAttempts=%u freeSuccesses=%u freeFailures=%u "
             "storageBytes=%u\n",
             loaded_entries, free_attempts, free_successes, free_failures, freed_storage_bytes);
}

bool InitializeOverrideCache()
{
    DbgPrint("[codxe][assets] InitializeOverrideCache begin. existing=%p capacity=%u\n", s_override_cache,
             static_cast<unsigned int>(s_override_cache_capacity));

    if (s_override_cache != nullptr)
    {
        ClearOverrideCache();
        DbgPrint("[codxe][assets] InitializeOverrideCache end. reused existing cache.\n");
        return true;
    }

    const size_t cache_size = sizeof(OverrideCacheEntry) * MAX_OVERRIDE_CACHE_ENTRIES;
    s_override_cache = static_cast<OverrideCacheEntry *>(AllocVirtualBlock(cache_size, "override cache metadata"));
    if (s_override_cache == nullptr)
    {
        Com_Printf(CON_CHANNEL_FILES,
                   "[codxe][assets] Failed to allocate override cache. entries=%u bytes=%u error=0x%08X\n",
                   static_cast<unsigned int>(MAX_OVERRIDE_CACHE_ENTRIES), static_cast<unsigned int>(cache_size),
                   GetLastError());
        DbgPrint("[codxe][assets] InitializeOverrideCache end. failed bytes=%u error=0x%08X\n",
                 static_cast<unsigned int>(cache_size), GetLastError());
        s_override_cache_capacity = 0;
        return false;
    }

    s_override_cache_capacity = MAX_OVERRIDE_CACHE_ENTRIES;
    memset(s_override_cache, 0, cache_size);

    Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Override cache allocated. entries=%u bytes=%u storage=%p\n",
               static_cast<unsigned int>(s_override_cache_capacity), static_cast<unsigned int>(cache_size),
               s_override_cache);
    DbgPrint("[codxe][assets] InitializeOverrideCache end. cache=%p bytes=%u\n", s_override_cache,
             static_cast<unsigned int>(cache_size));
    return true;
}

void ShutdownOverrideCache()
{
    DbgPrint("[codxe][assets] ShutdownOverrideCache begin. cache=%p capacity=%u\n", s_override_cache,
             static_cast<unsigned int>(s_override_cache_capacity));

    const DWORD cache_bytes = static_cast<DWORD>(sizeof(OverrideCacheEntry) * s_override_cache_capacity);
    ClearOverrideCache();

    if (s_override_cache != nullptr)
    {
        if (FreeVirtualBlock(s_override_cache, cache_bytes, "override cache metadata"))
        {
            DbgPrint("[codxe][assets] ShutdownOverrideCache freed cache metadata. bytes=%u\n", cache_bytes);
        }
        else
        {
            DbgPrint("[codxe][assets] ShutdownOverrideCache failed to free cache metadata. bytes=%u\n", cache_bytes);
        }
    }

    s_override_cache = nullptr;
    s_override_cache_capacity = 0;
    DbgPrint("[codxe][assets] ShutdownOverrideCache end.\n");
}

bool CopyAssetName(char *dest, size_t dest_size, const char *name)
{
    if (dest == nullptr || dest_size == 0 || name == nullptr || name[0] == '\0')
    {
        return false;
    }

    size_t i = 0;
    for (; name[i] != '\0'; ++i)
    {
        if (i + 1 >= dest_size)
        {
            dest[0] = '\0';
            return false;
        }

        dest[i] = name[i];
    }

    dest[i] = '\0';
    return true;
}

bool AppendPathPart(char *path, size_t path_size, const char *text, bool normalize_slashes)
{
    if (path == nullptr || path_size == 0 || text == nullptr)
    {
        return false;
    }

    size_t length = strlen(path);
    for (size_t i = 0; text[i] != '\0'; ++i)
    {
        if (length + 1 >= path_size)
        {
            return false;
        }

        char c = text[i];
        if (normalize_slashes && c == '/')
        {
            c = '\\';
        }

        path[length++] = c;
    }

    path[length] = '\0';
    return true;
}

bool BuildAssetPath(char *path, size_t path_size, const char *asset_name, const char *extension = nullptr)
{
    if (path == nullptr || path_size == 0)
    {
        return false;
    }

    path[0] = '\0';

    const char *base_path = Config::GetModBasePathCStr();
    if (base_path == nullptr || base_path[0] == '\0' || asset_name == nullptr || asset_name[0] == '\0')
    {
        return false;
    }

    if (!AppendPathPart(path, path_size, base_path, false))
    {
        return false;
    }

    if (!AppendPathPart(path, path_size, "\\", false))
    {
        return false;
    }

    if (!AppendPathPart(path, path_size, asset_name, true))
    {
        return false;
    }

    if (extension != nullptr && !AppendPathPart(path, path_size, extension, false))
    {
        return false;
    }

    return true;
}

OverrideCacheEntry *FindCacheEntry(XAssetType type, const char *name)
{
    if (name == nullptr || name[0] == '\0')
    {
        return nullptr;
    }

    if (s_override_cache == nullptr)
    {
        return nullptr;
    }

    for (size_t i = 0; i < s_override_cache_capacity; ++i)
    {
        OverrideCacheEntry &entry = s_override_cache[i];
        if (entry.state == CACHE_LOADED && entry.type == type && strcmp(entry.name, name) == 0)
        {
            return &entry;
        }
    }

    return nullptr;
}

OverrideCacheEntry *AllocateCacheEntry(XAssetType type, const char *name)
{
    if (s_override_cache == nullptr)
    {
        return nullptr;
    }

    for (size_t i = 0; i < s_override_cache_capacity; ++i)
    {
        if (s_override_cache[i].state == CACHE_EMPTY)
        {
            OverrideCacheEntry *entry = &s_override_cache[i];
            memset(entry, 0, sizeof(*entry));
            entry->type = type;
            if (!CopyAssetName(entry->name, sizeof(entry->name), name))
            {
                memset(entry, 0, sizeof(*entry));
                return nullptr;
            }

            return entry;
        }
    }

    Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Override cache full. type=%d name='%s'\n", type, name);
    return nullptr;
}

void *AllocStorage(size_t size, const char *reason)
{
    if (size == 0 || size > 0x7FFFFFFF)
    {
        Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] VirtualAlloc rejected for '%s'. size=%u\n",
                   reason ? reason : "<null>", static_cast<unsigned int>(size));
        return nullptr;
    }

    void *storage = AllocVirtualBlock(size, reason);
    if (storage == nullptr)
    {
        Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] VirtualAlloc failed for '%s'. size=%u error=0x%08X\n",
                   reason ? reason : "<null>", static_cast<unsigned int>(size), GetLastError());
    }

    return storage;
}

bool FileExistsFast(const char *path)
{
    const DWORD attrs = GetFileAttributesA(path);
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool LoadFileBuffer(const char *path, bool null_terminate, void *&storage, DWORD &storage_size, DWORD &payload_size)
{
    storage = nullptr;
    storage_size = 0;
    payload_size = 0;

    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    const DWORD file_size = GetFileSize(file, nullptr);
    if (file_size == INVALID_FILE_SIZE || file_size > 0x7FFFFFFF)
    {
        CloseHandle(file);
        return false;
    }

    const DWORD extra_byte = null_terminate ? 1 : 0;
    const size_t alloc_size = static_cast<size_t>(file_size) + extra_byte;
    storage = AllocStorage(alloc_size == 0 ? 1 : alloc_size, path);
    if (storage == nullptr)
    {
        CloseHandle(file);
        return false;
    }

    if (file_size > 0)
    {
        DWORD bytes_read = 0;
        if (!ReadFile(file, storage, file_size, &bytes_read, nullptr) || bytes_read != file_size)
        {
            CloseHandle(file);
            FreeVirtualBlock(storage, static_cast<DWORD>(alloc_size == 0 ? 1 : alloc_size), path);
            storage = nullptr;
            return false;
        }
    }

    CloseHandle(file);

    if (null_terminate)
    {
        static_cast<char *>(storage)[file_size] = '\0';
    }

    storage_size = static_cast<DWORD>(alloc_size == 0 ? 1 : alloc_size);
    payload_size = file_size;
    return true;
}

enum CsvReaderState
{
    CSV_START_OF_FIELD,
    CSV_IN_FIELD,
    CSV_IN_QUOTED_FIELD,
    CSV_IN_ESCAPED_QUOTE,
    CSV_END_OF_ROW,
    CSV_EMPTY
};

struct CsvReader
{
    const char *data;
    size_t size;
    size_t cursor;
    CsvReaderState state;
};

struct CsvStats
{
    size_t row_count;
    size_t column_count;
    size_t field_count;
    size_t string_bytes;
};

bool IsCsvTerminator(char c)
{
    return c == '\r' || c == '\n';
}

void SkipCsvLfAfterCr(CsvReader &reader, char c)
{
    if (c == '\r' && reader.cursor < reader.size && reader.data[reader.cursor] == '\n')
    {
        ++reader.cursor;
    }
}

bool AppendCsvChar(char *output, char *output_end, size_t &field_length, char c)
{
    if (field_length >= 0x7FFFFFFF)
    {
        return false;
    }

    if (output != nullptr)
    {
        if (output + field_length >= output_end)
        {
            return false;
        }

        output[field_length] = c;
    }

    ++field_length;
    return true;
}

bool TerminateCsvField(char *output, char *output_end, size_t field_length)
{
    if (output == nullptr)
    {
        return true;
    }

    if (output + field_length >= output_end)
    {
        return false;
    }

    output[field_length] = '\0';
    return true;
}

bool ReadCsvField(CsvReader &reader, char *output, char *output_end, size_t &field_length, bool &row_end,
                  bool &csv_end)
{
    field_length = 0;
    row_end = false;
    csv_end = false;

    if (reader.state == CSV_EMPTY)
    {
        csv_end = true;
        return true;
    }

    if (reader.state == CSV_END_OF_ROW)
    {
        reader.state = CSV_START_OF_FIELD;
        row_end = true;
        return true;
    }

    bool field_started = false;
    for (;;)
    {
        if (reader.cursor >= reader.size)
        {
            reader.state = CSV_EMPTY;
            if (!field_started && field_length == 0)
            {
                csv_end = true;
                return true;
            }

            return TerminateCsvField(output, output_end, field_length);
        }

        const char c = reader.data[reader.cursor++];
        field_started = true;

        switch (reader.state)
        {
        case CSV_START_OF_FIELD:
            if (IsCsvTerminator(c))
            {
                SkipCsvLfAfterCr(reader, c);
                reader.state = CSV_END_OF_ROW;
                return TerminateCsvField(output, output_end, field_length);
            }

            if (c == '"')
            {
                reader.state = CSV_IN_QUOTED_FIELD;
            }
            else if (c == ',')
            {
                return TerminateCsvField(output, output_end, field_length);
            }
            else
            {
                reader.state = CSV_IN_FIELD;
                if (!AppendCsvChar(output, output_end, field_length, c))
                {
                    return false;
                }
            }
            break;

        case CSV_IN_FIELD:
            if (IsCsvTerminator(c))
            {
                SkipCsvLfAfterCr(reader, c);
                reader.state = CSV_END_OF_ROW;
                return TerminateCsvField(output, output_end, field_length);
            }

            if (c == ',')
            {
                reader.state = CSV_START_OF_FIELD;
                return TerminateCsvField(output, output_end, field_length);
            }

            if (!AppendCsvChar(output, output_end, field_length, c))
            {
                return false;
            }
            break;

        case CSV_IN_QUOTED_FIELD:
            if (c == '"')
            {
                reader.state = CSV_IN_ESCAPED_QUOTE;
            }
            else if (!AppendCsvChar(output, output_end, field_length, c))
            {
                return false;
            }
            break;

        case CSV_IN_ESCAPED_QUOTE:
            if (IsCsvTerminator(c))
            {
                SkipCsvLfAfterCr(reader, c);
                reader.state = CSV_END_OF_ROW;
                return TerminateCsvField(output, output_end, field_length);
            }

            if (c == '"')
            {
                reader.state = CSV_IN_QUOTED_FIELD;
                if (!AppendCsvChar(output, output_end, field_length, c))
                {
                    return false;
                }
            }
            else if (c == ',')
            {
                reader.state = CSV_START_OF_FIELD;
                return TerminateCsvField(output, output_end, field_length);
            }
            else
            {
                reader.state = CSV_IN_FIELD;
                if (!AppendCsvChar(output, output_end, field_length, c))
                {
                    return false;
                }
            }
            break;

        default:
            return false;
        }
    }
}

bool AddCsvFieldToStats(CsvStats &stats, size_t &current_columns, size_t field_length)
{
    if (stats.field_count >= 0x7FFFFFFF || field_length > 0x7FFFFFFF ||
        stats.string_bytes > 0x7FFFFFFF - (field_length + 1))
    {
        return false;
    }

    ++current_columns;
    ++stats.field_count;
    stats.string_bytes += field_length + 1;
    return true;
}

bool FinishCsvStatsRow(CsvStats &stats, size_t &current_columns)
{
    if (stats.row_count >= 0x7FFFFFFF || current_columns > 0x7FFFFFFF)
    {
        return false;
    }

    ++stats.row_count;
    if (current_columns > stats.column_count)
    {
        stats.column_count = current_columns;
    }

    current_columns = 0;
    return true;
}

bool ScanStringTableCsvBuffer(const char *data, size_t data_size, CsvStats &stats)
{
    memset(&stats, 0, sizeof(stats));

    CsvReader reader = {data, data_size, 0, CSV_START_OF_FIELD};
    size_t current_columns = 0;
    for (;;)
    {
        size_t field_length = 0;
        bool row_end = false;
        bool csv_end = false;
        if (!ReadCsvField(reader, nullptr, nullptr, field_length, row_end, csv_end))
        {
            return false;
        }

        if (csv_end)
        {
            if (current_columns != 0 && !FinishCsvStatsRow(stats, current_columns))
            {
                return false;
            }

            break;
        }

        if (row_end)
        {
            if (!FinishCsvStatsRow(stats, current_columns))
            {
                return false;
            }

            continue;
        }

        if (!AddCsvFieldToStats(stats, current_columns, field_length))
        {
            return false;
        }
    }

    const size_t cell_count = stats.row_count * stats.column_count;
    if (stats.column_count != 0 && cell_count / stats.column_count != stats.row_count)
    {
        return false;
    }

    if (stats.field_count > cell_count)
    {
        return false;
    }

    const size_t padded_empty_fields = cell_count - stats.field_count;
    if (stats.string_bytes > 0x7FFFFFFF - padded_empty_fields)
    {
        return false;
    }

    stats.string_bytes += padded_empty_fields;
    return true;
}

bool WriteEmptyStringCell(const char **values, size_t cell_index, char *&string_cursor, char *string_end)
{
    if (string_cursor >= string_end)
    {
        return false;
    }

    values[cell_index] = string_cursor;
    *string_cursor++ = '\0';
    return true;
}

bool FinishStringTableRow(const char **values, size_t row_count, size_t column_count, size_t &row_index,
                          size_t &current_column, char *&string_cursor, char *string_end)
{
    if (row_index >= row_count)
    {
        return false;
    }

    while (current_column < column_count)
    {
        if (!WriteEmptyStringCell(values, row_index * column_count + current_column, string_cursor, string_end))
        {
            return false;
        }

        ++current_column;
    }

    ++row_index;
    current_column = 0;
    return true;
}

bool FillStringTableCsvBuffer(const char *data, size_t data_size, void *storage, size_t row_count, size_t column_count,
                              size_t values_bytes_aligned, size_t string_bytes)
{
    const size_t cell_count = row_count * column_count;
    if (cell_count == 0)
    {
        return row_count == 0 && column_count == 0;
    }

    const char **values = reinterpret_cast<const char **>(storage);
    char *string_cursor = static_cast<char *>(storage) + values_bytes_aligned;
    char *string_end = string_cursor + string_bytes;
    CsvReader reader = {data, data_size, 0, CSV_START_OF_FIELD};
    size_t row_index = 0;
    size_t current_column = 0;

    for (;;)
    {
        char *field_start = string_cursor;
        size_t field_length = 0;
        bool row_end = false;
        bool csv_end = false;
        if (!ReadCsvField(reader, field_start, string_end, field_length, row_end, csv_end))
        {
            return false;
        }

        if (csv_end)
        {
            if (current_column != 0 &&
                !FinishStringTableRow(values, row_count, column_count, row_index, current_column, string_cursor,
                                      string_end))
            {
                return false;
            }

            return row_index == row_count;
        }

        if (row_end)
        {
            if (!FinishStringTableRow(values, row_count, column_count, row_index, current_column, string_cursor,
                                      string_end))
            {
                return false;
            }

            continue;
        }

        if (row_index >= row_count || current_column >= column_count)
        {
            return false;
        }

        values[row_index * column_count + current_column] = field_start;
        string_cursor = field_start + field_length + 1;
        ++current_column;
    }
}

StringTable *LoadStringTableOverride(const char *asset_name)
{
    OverrideCacheEntry *entry = FindCacheEntry(ASSET_TYPE_STRINGTABLE, asset_name);
    if (entry != nullptr)
    {
        return &entry->stringtable;
    }

    char path[MAX_PATH];
    if (!BuildAssetPath(path, sizeof(path), asset_name))
    {
        return nullptr;
    }

    if (!FileExistsFast(path))
    {
        return nullptr;
    }

    void *file_storage = nullptr;
    DWORD file_storage_size = 0;
    DWORD file_size = 0;
    if (!LoadFileBuffer(path, true, file_storage, file_storage_size, file_size))
    {
        return nullptr;
    }

    CsvStats stats;
    if (!ScanStringTableCsvBuffer(static_cast<const char *>(file_storage), file_size, stats))
    {
        Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Failed to parse stringtable csv '%s'\n", path);
        FreeVirtualBlock(file_storage, file_storage_size, path);
        return nullptr;
    }

    const size_t cell_count = stats.row_count * stats.column_count;
    const size_t values_bytes = cell_count * sizeof(const char *);
    const size_t values_bytes_aligned = AlignSize(values_bytes, STRINGTABLE_VALUE_ALIGNMENT);
    if (cell_count > 0x7FFFFFFF || values_bytes_aligned > 0x7FFFFFFF ||
        stats.string_bytes > 0x7FFFFFFF - values_bytes_aligned)
    {
        Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Stringtable csv too large '%s'\n", path);
        FreeVirtualBlock(file_storage, file_storage_size, path);
        return nullptr;
    }

    void *storage = nullptr;
    const size_t storage_size = values_bytes_aligned + stats.string_bytes;
    if (storage_size > 0)
    {
        storage = AllocStorage(storage_size, path);
        if (storage == nullptr)
        {
            FreeVirtualBlock(file_storage, file_storage_size, path);
            return nullptr;
        }

        memset(storage, 0, storage_size);
    }

    if (storage_size > 0 &&
        !FillStringTableCsvBuffer(static_cast<const char *>(file_storage), file_size, storage, stats.row_count,
                                  stats.column_count, values_bytes_aligned, stats.string_bytes))
    {
        Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Failed to build stringtable csv '%s'\n", path);
        FreeVirtualBlock(file_storage, file_storage_size, path);
        FreeVirtualBlock(storage, static_cast<DWORD>(storage_size), path);
        return nullptr;
    }

    FreeVirtualBlock(file_storage, file_storage_size, path);

    entry = AllocateCacheEntry(ASSET_TYPE_STRINGTABLE, asset_name);
    if (entry == nullptr)
    {
        if (storage != nullptr)
        {
            FreeVirtualBlock(storage, static_cast<DWORD>(storage_size), path);
        }

        return nullptr;
    }

    entry->storage = storage;
    entry->storage_size = static_cast<DWORD>(storage_size);
    entry->payload_size = static_cast<DWORD>(storage_size);
    entry->stringtable.name = entry->name;
    entry->stringtable.columnCount = static_cast<int>(stats.column_count);
    entry->stringtable.rowCount = static_cast<int>(stats.row_count);
    entry->stringtable.values = cell_count == 0 ? nullptr : reinterpret_cast<const char **>(storage);
    entry->state = CACHE_LOADED;

    Com_Printf(CON_CHANNEL_FILES,
               "[codxe][assets] Loaded stringtable override '%s' from '%s'. rows=%u columns=%u storage=%p size=%u\n",
               asset_name, path, static_cast<unsigned int>(stats.row_count),
               static_cast<unsigned int>(stats.column_count), entry->storage, entry->storage_size);

    return &entry->stringtable;
}

OverrideCacheEntry *LoadMapEntsOverride(const char *asset_name)
{
    OverrideCacheEntry *entry = FindCacheEntry(ASSET_TYPE_MAP_ENTS, asset_name);
    if (entry != nullptr)
    {
        return entry;
    }

    char path[MAX_PATH];
    if (!BuildAssetPath(path, sizeof(path), asset_name, ".ents"))
    {
        return nullptr;
    }

    void *storage = nullptr;
    DWORD storage_size = 0;
    DWORD payload_size = 0;
    if (!LoadFileBuffer(path, true, storage, storage_size, payload_size))
    {
        return nullptr;
    }

    entry = AllocateCacheEntry(ASSET_TYPE_MAP_ENTS, asset_name);
    if (entry == nullptr)
    {
        FreeVirtualBlock(storage, storage_size, path);
        return nullptr;
    }

    entry->storage = storage;
    entry->storage_size = storage_size;
    entry->payload_size = payload_size;
    entry->state = CACHE_LOADED;

    Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Loaded map ents override '%s' from '%s'. storage=%p size=%u\n",
               asset_name, path, entry->storage, entry->payload_size);

    return entry;
}

void OverrideMapEnts(MapEnts *asset)
{
    if (asset == nullptr || asset->name == nullptr || asset->name[0] == '\0')
    {
        return;
    }

    OverrideCacheEntry *entry = LoadMapEntsOverride(asset->name);
    if (entry == nullptr || entry->state != CACHE_LOADED)
    {
        return;
    }

    asset->entityString = static_cast<char *>(entry->storage);
    asset->numEntityChars = static_cast<int>(entry->payload_size);

    Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Override applied. type=mapents name='%s'\n", asset->name);
}

void OverrideStringTable(StringTable *asset)
{
    if (asset == nullptr || asset->name == nullptr || asset->name[0] == '\0')
    {
        return;
    }

    StringTable *override_asset = LoadStringTableOverride(asset->name);
    if (override_asset == nullptr)
    {
        return;
    }

    asset->columnCount = override_asset->columnCount;
    asset->rowCount = override_asset->rowCount;
    asset->values = override_asset->values;

    Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Override applied. type=stringtable name='%s'\n", asset->name);
}

XAssetEntry *DB_LinkXAssetEntry_Hook(XAssetEntry *newEntry, int allowOverride)
{
    if (newEntry != nullptr)
    {
        switch (newEntry->asset.type)
        {
        case ASSET_TYPE_MAP_ENTS:
            OverrideMapEnts(newEntry->asset.header.mapEnts);
            break;
        case ASSET_TYPE_STRINGTABLE:
            OverrideStringTable(newEntry->asset.header.stringTable);
            break;
        }
    }

    return DB_LinkXAssetEntry_Detour.GetOriginal<DB_LinkXAssetEntry_t>()(newEntry, allowOverride);
}

} // namespace

assets::assets()
{
    DbgPrint("[codxe][assets] ctor begin.\n");
    InitializeOverrideCache();

    DB_LinkXAssetEntry_Detour = Detour(DB_LinkXAssetEntry, DB_LinkXAssetEntry_Hook);
    DB_LinkXAssetEntry_Detour.Install();
    DbgPrint("[codxe][assets] ctor end.\n");
}

assets::~assets()
{
    DbgPrint("[codxe][assets] dtor begin.\n");
    DB_LinkXAssetEntry_Detour.Remove();

    ShutdownOverrideCache();
    DbgPrint("[codxe][assets] dtor end.\n");
}
} // namespace mp
} // namespace iw3
