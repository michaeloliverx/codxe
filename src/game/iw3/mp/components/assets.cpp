#include "pch.h"
#include "assets.h"
#include "third_party/aria_csv/csv_parser.hpp"
#include <unordered_map>

namespace iw3
{
namespace mp
{

namespace Assets
{
namespace Utils
{
std::string BuildAssetPath(const char *asset_name, const char *extension = nullptr)
{
    const std::string base_path = Config::GetModBasePath();
    if (base_path.empty() || asset_name == nullptr || asset_name[0] == '\0')
    {
        return "";
    }

    std::string relative_path = asset_name;
    std::replace(relative_path.begin(), relative_path.end(), '/', '\\');

    std::string path = base_path + "\\" + relative_path;
    if (extension != nullptr)
    {
        path += extension;
    }

    return path;
}

bool ReadAssetOverride(const std::string &path, std::string &buffer)
{
    buffer.clear();

    HANDLE file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
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

    if (file_size > 0)
    {
        buffer.resize(file_size);

        DWORD bytes_read = 0;
        if (!ReadFile(file, &buffer[0], file_size, &bytes_read, nullptr) || bytes_read != file_size)
        {
            CloseHandle(file);
            buffer.clear();
            return false;
        }
    }

    CloseHandle(file);
    return true;
}

} // namespace Utils

namespace MapEnts_
{
std::unordered_map<std::string, std::unique_ptr<std::string>> mapents_buffers;

void _override(MapEnts *asset)
{
    if (!asset || !asset->name || asset->name[0] == '\0')
    {
        return;
    }

    const std::string path = Utils::BuildAssetPath(asset->name, ".ents");
    if (path.empty())
    {
        return;
    }

    std::string buffer;
    if (!Utils::ReadAssetOverride(path, buffer))
    {
        return;
    }

    Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Overriding map ents '%s' from '%s'\n", asset->name, path.c_str());

    auto itr = mapents_buffers.find(asset->name);
    if (itr != mapents_buffers.end())
    {
        mapents_buffers.erase(itr);
    }

    mapents_buffers[asset->name] = make_unique<std::string>();
    itr = mapents_buffers.find(asset->name);
    std::string *mapents_buffer = itr->second.get();

    mapents_buffer->assign(buffer);

    asset->entityString = const_cast<char *>(mapents_buffer->c_str());
    asset->numEntityChars = static_cast<int>(mapents_buffer->length());
}
} // namespace MapEnts_

namespace RawFile_
{
struct RawFileOverride
{
    std::string name;
    std::string buffer;
    RawFile asset;
};

std::unordered_map<std::string, std::unique_ptr<RawFileOverride>> rawfile_overrides;

std::string BuildRawFilePath(const char *asset_name)
{
    return Utils::BuildAssetPath(asset_name);
}

RawFile *FindOverride(const char *asset_name)
{
    if (asset_name == nullptr || asset_name[0] == '\0')
    {
        return nullptr;
    }

    auto itr = rawfile_overrides.find(asset_name);
    if (itr != rawfile_overrides.end())
    {
        return itr->second ? &itr->second->asset : nullptr;
    }

    const std::string path = BuildRawFilePath(asset_name);
    if (path.empty())
    {
        rawfile_overrides[asset_name] = nullptr;
        return nullptr;
    }

    std::string buffer;
    if (!Utils::ReadAssetOverride(path, buffer))
    {
        rawfile_overrides[asset_name] = nullptr;
        return nullptr;
    }

    Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Loaded rawfile '%s' from '%s'\n", asset_name, path.c_str());

    rawfile_overrides[asset_name] = make_unique<RawFileOverride>();
    itr = rawfile_overrides.find(asset_name);
    RawFileOverride *rawfile_override = itr->second.get();

    rawfile_override->name = asset_name;
    rawfile_override->buffer.assign(buffer);
    rawfile_override->asset.name = rawfile_override->name.c_str();
    rawfile_override->asset.len = static_cast<int>(rawfile_override->buffer.length());
    rawfile_override->asset.buffer = rawfile_override->buffer.c_str();

    return &rawfile_override->asset;
}

void _override(RawFile *asset)
{
    if (!asset || !asset->name || asset->name[0] == '\0')
    {
        return;
    }

    RawFile *override_asset = FindOverride(asset->name);
    if (override_asset == nullptr)
    {
        return;
    }

    Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Overriding rawfile '%s'\n", asset->name);
    asset->len = override_asset->len;
    asset->buffer = override_asset->buffer;
}
} // namespace RawFile_

namespace StringTable_
{
struct StringTableOverride
{
    std::string name;
    std::vector<std::string> cells;
    std::vector<const char *> values;
    StringTable asset;
};

std::unordered_map<std::string, std::unique_ptr<StringTableOverride>> stringtable_overrides;

bool LoadCsv(const std::string &path, std::vector<std::vector<std::string>> &rows)
{
    rows.clear();

    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
    {
        return false;
    }

    try
    {
        aria::csv::CsvParser parser(file);
        for (aria::csv::CsvParser::iterator itr = parser.begin(); itr != parser.end(); ++itr)
        {
            rows.push_back(*itr);
        }
    }
    catch (...)
    {
        Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Failed to parse stringtable csv '%s'\n", path.c_str());
        rows.clear();
        return false;
    }

    return true;
}

StringTable *FindOverride(const char *asset_name)
{
    if (asset_name == nullptr || asset_name[0] == '\0')
    {
        return nullptr;
    }

    auto itr = stringtable_overrides.find(asset_name);
    if (itr != stringtable_overrides.end())
    {
        return itr->second ? &itr->second->asset : nullptr;
    }

    const std::string path = Utils::BuildAssetPath(asset_name);
    if (path.empty())
    {
        stringtable_overrides[asset_name] = nullptr;
        return nullptr;
    }

    std::vector<std::vector<std::string>> rows;
    if (!LoadCsv(path, rows))
    {
        stringtable_overrides[asset_name] = nullptr;
        return nullptr;
    }

    size_t column_count = 0;
    for (size_t row = 0; row < rows.size(); ++row)
    {
        if (rows[row].size() > column_count)
        {
            column_count = rows[row].size();
        }
    }

    const size_t cell_count = rows.size() * column_count;
    if (rows.size() > 0x7FFFFFFF || column_count > 0x7FFFFFFF || cell_count > 0x7FFFFFFF)
    {
        Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Stringtable csv too large '%s'\n", path.c_str());
        stringtable_overrides[asset_name] = nullptr;
        return nullptr;
    }

    Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Loaded stringtable '%s' from '%s'\n", asset_name, path.c_str());

    stringtable_overrides[asset_name] = make_unique<StringTableOverride>();
    itr = stringtable_overrides.find(asset_name);
    StringTableOverride *stringtable_override = itr->second.get();

    stringtable_override->name = asset_name;
    stringtable_override->cells.resize(cell_count);
    stringtable_override->values.resize(cell_count);

    for (size_t row = 0; row < rows.size(); ++row)
    {
        for (size_t column = 0; column < column_count; ++column)
        {
            const size_t cell_index = row * column_count + column;
            if (column < rows[row].size())
            {
                stringtable_override->cells[cell_index] = rows[row][column];
            }
        }
    }

    for (size_t i = 0; i < cell_count; ++i)
    {
        stringtable_override->values[i] = stringtable_override->cells[i].c_str();
    }

    stringtable_override->asset.name = stringtable_override->name.c_str();
    stringtable_override->asset.columnCount = static_cast<int>(column_count);
    stringtable_override->asset.rowCount = static_cast<int>(rows.size());
    stringtable_override->asset.values =
        stringtable_override->values.empty() ? nullptr : &stringtable_override->values[0];

    return &stringtable_override->asset;
}

void _override(StringTable *asset)
{
    if (!asset || !asset->name || asset->name[0] == '\0')
    {
        return;
    }

    StringTable *override_asset = FindOverride(asset->name);
    if (override_asset == nullptr)
    {
        return;
    }

    Com_Printf(CON_CHANNEL_FILES, "[codxe][assets] Overriding stringtable '%s'\n", asset->name);
    asset->columnCount = override_asset->columnCount;
    asset->rowCount = override_asset->rowCount;
    asset->values = override_asset->values;
}
} // namespace StringTable_
} // namespace Assets

Detour DB_LinkXAssetEntry_Detour;
Detour DB_FindXAssetHeader_Detour;

XAssetEntry *DB_LinkXAssetEntry_Hook(XAssetEntry *newEntry, int allowOverride)
{
    if (newEntry != nullptr)
    {
        switch (newEntry->asset.type)
        {
        case ASSET_TYPE_MAP_ENTS:
            Assets::MapEnts_::_override(newEntry->asset.header.mapEnts);
            break;
        case ASSET_TYPE_RAWFILE:
            Assets::RawFile_::_override(newEntry->asset.header.rawfile);
            break;
        case ASSET_TYPE_STRINGTABLE:
            Assets::StringTable_::_override(newEntry->asset.header.stringTable);
            break;
        }
    }

    return DB_LinkXAssetEntry_Detour.GetOriginal<DB_LinkXAssetEntry_t>()(newEntry, allowOverride);
}

XAssetHeader DB_FindXAssetHeader_Hook(const XAssetType type, const char *name)
{
    if (type == ASSET_TYPE_RAWFILE)
    {
        RawFile *rawfile = Assets::RawFile_::FindOverride(name);
        if (rawfile != nullptr)
        {
            XAssetHeader header;
            header.rawfile = rawfile;
            return header;
        }
    }
    else if (type == ASSET_TYPE_STRINGTABLE)
    {
        StringTable *stringtable = Assets::StringTable_::FindOverride(name);
        if (stringtable != nullptr)
        {
            XAssetHeader header;
            header.stringTable = stringtable;
            return header;
        }
    }

    return DB_FindXAssetHeader_Detour.GetOriginal<DB_FindXAssetHeader_t>()(type, name);
}

assets::assets()
{
    DB_LinkXAssetEntry_Detour = Detour(DB_LinkXAssetEntry, DB_LinkXAssetEntry_Hook);
    DB_LinkXAssetEntry_Detour.Install();

    DB_FindXAssetHeader_Detour = Detour(DB_FindXAssetHeader, DB_FindXAssetHeader_Hook);
    DB_FindXAssetHeader_Detour.Install();
}

assets::~assets()
{
    DB_FindXAssetHeader_Detour.Remove();

    DB_LinkXAssetEntry_Detour.Remove();
}
} // namespace mp
} // namespace iw3
