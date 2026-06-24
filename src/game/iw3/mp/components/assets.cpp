#include <streambuf>

#include "pch.h"
#include "assets.h"
#include "third_party/aria_csv/csv_parser.hpp"


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
    DWORD storage_process_type;
    DWORD payload_size;
    StringTable stringtable;
};

OverrideCacheEntry *s_override_cache = nullptr;
size_t s_override_cache_capacity = 0;

size_t AlignSize(size_t value, size_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

void *AllocVirtualBlock(size_t size, const char *reason)
{
    void *storage = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    UNREFERENCED_PARAMETER(reason);
    return storage;
}

bool FreeVirtualBlock(void *storage, DWORD size, const char *reason)
{
    if (storage == nullptr)
    {
        return true;
    }

    const BOOL result = VirtualFree(storage, 0, MEM_RELEASE);
    UNREFERENCED_PARAMETER(size);
    UNREFERENCED_PARAMETER(reason);
    return result != FALSE;
}

void ResetCacheEntry(OverrideCacheEntry &entry)
{
    if (entry.storage != nullptr)
    {
        const DWORD current_process_type = KeGetCurrentProcessType();
        if (entry.storage_process_type == 0 || entry.storage_process_type == current_process_type)
        {
            FreeVirtualBlock(entry.storage, entry.storage_size, entry.name);
        }
    }

    memset(&entry, 0, sizeof(entry));
}

void ClearOverrideCache()
{
    if (s_override_cache == nullptr)
    {
        return;
    }

    for (size_t i = 0; i < s_override_cache_capacity; ++i)
    {
        ResetCacheEntry(s_override_cache[i]);
    }
}

bool InitializeOverrideCache()
{
    if (s_override_cache != nullptr)
    {
        ClearOverrideCache();
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
        s_override_cache_capacity = 0;
        return false;
    }

    s_override_cache_capacity = MAX_OVERRIDE_CACHE_ENTRIES;
    memset(s_override_cache, 0, cache_size);
    return true;
}

void ShutdownOverrideCache()
{
    const DWORD cache_bytes = static_cast<DWORD>(sizeof(OverrideCacheEntry) * s_override_cache_capacity);
    ClearOverrideCache();

    if (s_override_cache != nullptr)
    {
        if (!FreeVirtualBlock(s_override_cache, cache_bytes, "override cache metadata"))
        {
            Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Failed to free override cache metadata. bytes=%u\n",
                       cache_bytes);
        }
    }

    s_override_cache = nullptr;
    s_override_cache_capacity = 0;
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

bool BuildSourceDisplayPath(char *path, size_t path_size, const char *asset_name, const char *extension = nullptr)
{
    if (path == nullptr || path_size == 0)
    {
        return false;
    }

    path[0] = '\0';
    if (Config::active_mod.empty() || asset_name == nullptr || asset_name[0] == '\0')
    {
        return false;
    }

    if (!AppendPathPart(path, path_size, "mods\\", false))
    {
        return false;
    }

    if (!AppendPathPart(path, path_size, Config::active_mod.c_str(), false))
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

void PrintOverrideApplied(const char *type, const char *asset_name, const char *source)
{
    char display_name[MAX_PATH];
    display_name[0] = '\0';
    const char *name = asset_name;
    if (AppendPathPart(display_name, sizeof(display_name), asset_name, true))
    {
        name = display_name;
    }

    Com_Printf(CON_CHANNEL_FILES, "^2codxe^7: %s \"%s\" -> \"%s\"\n", type, name, source);
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
    return attrs != INVALID_FILE_SIZE && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
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

struct CsvStats
{
    size_t row_count;
    size_t column_count;
    size_t field_count;
    size_t string_bytes;
};

class MemoryCsvStreamBuf : public std::streambuf
{
  public:
    MemoryCsvStreamBuf(const char *data, size_t size)
    {
        char *begin = const_cast<char *>(data);
        setg(begin, begin, begin + size);
    }
};

bool AddCsvFieldToStats(CsvStats &stats, size_t &current_columns, const std::string &field)
{
    const size_t field_length = field.size();
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

bool FinalizeCsvStats(CsvStats &stats)
{
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

bool ScanStringTableCsvBuffer(const char *data, size_t data_size, CsvStats &stats)
{
    memset(&stats, 0, sizeof(stats));

    MemoryCsvStreamBuf streambuf(data, data_size);
    std::istream input(&streambuf);

    try
    {
        aria::csv::CsvParser parser(input);
        size_t current_columns = 0;

        for (;;)
        {
            const aria::csv::Field field = parser.next_field();
            switch (field.type)
            {
            case aria::csv::FieldType::CSV_END:
                if (current_columns != 0 && !FinishCsvStatsRow(stats, current_columns))
                {
                    return false;
                }

                return FinalizeCsvStats(stats);

            case aria::csv::FieldType::ROW_END:
                if (!FinishCsvStatsRow(stats, current_columns))
                {
                    return false;
                }

                break;

            case aria::csv::FieldType::DATA:
                if (field.data == nullptr || !AddCsvFieldToStats(stats, current_columns, *field.data))
                {
                    return false;
                }

                break;
            }
        }
    }
    catch (const std::exception &e)
    {
        Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] aria csv scan exception: %s\n", e.what());
        return false;
    }
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

bool WriteStringTableField(const std::string &field, const char **values, size_t row_count, size_t column_count,
                           size_t row_index, size_t &current_column, char *&string_cursor, char *string_end)
{
    if (row_index >= row_count || current_column >= column_count)
    {
        return false;
    }

    const size_t field_length = field.size();
    if (field_length > 0x7FFFFFFF)
    {
        return false;
    }

    const size_t remaining = static_cast<size_t>(string_end - string_cursor);
    if (remaining < field_length + 1)
    {
        return false;
    }

    values[row_index * column_count + current_column] = string_cursor;
    if (field_length > 0)
    {
        memcpy(string_cursor, field.c_str(), field_length);
    }

    string_cursor[field_length] = '\0';
    string_cursor += field_length + 1;
    ++current_column;
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
    MemoryCsvStreamBuf streambuf(data, data_size);
    std::istream input(&streambuf);
    size_t row_index = 0;
    size_t current_column = 0;

    try
    {
        aria::csv::CsvParser parser(input);

        for (;;)
        {
            const aria::csv::Field field = parser.next_field();
            switch (field.type)
            {
            case aria::csv::FieldType::CSV_END:
                if (current_column != 0 && !FinishStringTableRow(values, row_count, column_count, row_index,
                                                                 current_column, string_cursor, string_end))
                {
                    return false;
                }

                return row_index == row_count;

            case aria::csv::FieldType::ROW_END:
                if (!FinishStringTableRow(values, row_count, column_count, row_index, current_column, string_cursor,
                                          string_end))
                {
                    return false;
                }

                break;

            case aria::csv::FieldType::DATA:
                if (field.data == nullptr ||
                    !WriteStringTableField(*field.data, values, row_count, column_count, row_index, current_column,
                                           string_cursor, string_end))
                {
                    return false;
                }

                break;
            }
        }
    }
    catch (const std::exception &e)
    {
        Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] aria csv fill exception: %s\n", e.what());
        return false;
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
    entry->storage_process_type = KeGetCurrentProcessType();
    entry->payload_size = static_cast<DWORD>(storage_size);
    entry->stringtable.name = entry->name;
    entry->stringtable.columnCount = static_cast<int>(stats.column_count);
    entry->stringtable.rowCount = static_cast<int>(stats.row_count);
    entry->stringtable.values = cell_count == 0 ? nullptr : reinterpret_cast<const char **>(storage);
    entry->state = CACHE_LOADED;

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
    entry->storage_process_type = KeGetCurrentProcessType();
    entry->payload_size = payload_size;
    entry->state = CACHE_LOADED;

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

    char source[MAX_PATH];
    if (BuildSourceDisplayPath(source, sizeof(source), asset->name, ".ents"))
    {
        PrintOverrideApplied("mapents", asset->name, source);
    }
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

    char source[MAX_PATH];
    if (BuildSourceDisplayPath(source, sizeof(source), asset->name))
    {
        PrintOverrideApplied("stringtable", asset->name, source);
    }
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
    InitializeOverrideCache();

    DB_LinkXAssetEntry_Detour = Detour(DB_LinkXAssetEntry, DB_LinkXAssetEntry_Hook);
    DB_LinkXAssetEntry_Detour.Install();
}

assets::~assets()
{
    DB_LinkXAssetEntry_Detour.Remove();

    ShutdownOverrideCache();
}
} // namespace mp
} // namespace iw3
