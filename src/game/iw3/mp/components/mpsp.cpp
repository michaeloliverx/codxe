
/**
 * This module has everything necessary to load Singleplayer maps in Multiplayer.
 *  Inspiration
 * https://github.com/xoxor4d/iw3xo-dev/blob/4eeba0bf63fbf991f44be6eae105ea9e60df0c3f/src/components/modules/_map.cpp
 */

#include "pch.h"
#include "mpsp.h"
#include "unordered_map"

namespace iw3
{
namespace mp
{
bool mpsp::is_sp_map = false;

namespace FS
{

/*
 * Normalizes a file path by replacing forward slashes with backslashes,
 * removing duplicate backslashes, and trimming trailing backslashes (except for root paths).
 */
std::string NormalizePath(const std::string &path)
{
    std::string result = path;
    std::replace(result.begin(), result.end(), '/', '\\');

    size_t pos = 0;
    while ((pos = result.find("\\\\", pos)) != std::string::npos)
    {
        result.erase(pos, 1);
    }

    if (!result.empty() && result[result.length() - 1] == '\\')
    {
        size_t root_end = result.find_first_of('\\');
        if (root_end != result.length() - 1)
        {
            result.erase(result.length() - 1);
        }
    }

    return result;
}

bool WriteFile(const std::string &path, const char *data, int size)
{
    const std::string normalized = NormalizePath(path);

    // Create directories if they do not exist
    size_t pos = 0;
    while ((pos = normalized.find('\\', pos)) != std::string::npos)
    {
        std::string dir = normalized.substr(0, pos);
        CreateDirectoryA(dir.c_str(), NULL);
        pos++;
    }

    std::ofstream file(normalized.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        DbgPrint("[IO] Failed to open file for writing: %s\n", normalized.c_str());
        return false;
    }

    file.write(reinterpret_cast<const char *>(data), size);
    return file.good();
}

std::string ReadTextFile(const std::string &path)
{
    const std::string normalized = NormalizePath(path);
    std::ifstream file(normalized, std::ios::binary);
    if (!file)
    {
        DbgPrint("ReadTextFile: Failed to open file: %s\n", normalized.c_str());
        return "";
    }

    // Get file size and pre-allocate string
    file.seekg(0, std::ios::end);
    size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    std::string content(size, '\0');
    file.read(&content[0], size);

    return content;
}

std::vector<unsigned char> ReadBinaryFile(const std::string &path)
{
    const std::string normalized = NormalizePath(path);
    std::ifstream file(normalized, std::ios::binary);
    if (!file)
    {
        DbgPrint("[IO] ReadBinaryFile: Failed to open file: %s\n", normalized.c_str());
        return std::vector<unsigned char>();
    }

    // Get file size and pre-allocate vector
    file.seekg(0, std::ios::end);
    size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer(size);
    file.read(reinterpret_cast<char *>(buffer.data()), size);

    if (!file.good() && !file.eof())
    {
        DbgPrint("[IO] ReadBinaryFile: Error reading file: %s\n", normalized.c_str());
        return std::vector<unsigned char>();
    }

    return buffer;
}

} // namespace FS

namespace Asset
{

std::string get_load_dir()
{
    return "game:\\_codxe\\internal";
}
namespace MapEnts_
{

std::string mapents_buffer;

void override_(MapEnts *asset)
{
    // _load\maps\airplane.d3dbsp.ents
    const std::string filename = Asset::get_load_dir() + "\\" + asset->name + ".ents";
    const std::string buffer = FS::ReadTextFile(filename);

    if (buffer.empty())
    {
        return;
    }

    if (!mapents_buffer.empty())
    {
        mapents_buffer.clear();
    }

    DbgPrint("Overriding map_ents '%s'\n", asset->name);

    mapents_buffer.assign(buffer);

    asset->entityString = const_cast<char *>(mapents_buffer.c_str());
    asset->numEntityChars = mapents_buffer.length();
}

} // namespace MapEnts_

namespace RawFile_
{
std::unordered_map<std::string, std::unique_ptr<std::string>> rawfile_buffers;

void override_(RawFile *asset)
{
    if (!asset || !asset->name || asset->name[0] == '\0')
    {
        return;
    }

    const std::string filename = Asset::get_load_dir() + "\\" + asset->name;
    const std::string buffer = FS::ReadTextFile(filename);

    if (buffer.empty())
    {
        return;
    }

    DbgPrint("Overriding rawfile '%s'\n", asset->name);

    auto itr = rawfile_buffers.find(asset->name);
    if (itr != rawfile_buffers.end())
    {
        rawfile_buffers.erase(itr);
    }
    rawfile_buffers[asset->name] = make_unique<std::string>();
    itr = rawfile_buffers.find(asset->name);
    auto rawfile_buffer = itr->second.get();

    rawfile_buffer->assign(buffer);

    asset->len = rawfile_buffer->length();
    asset->buffer = rawfile_buffer->data();
}
} // namespace RawFile_

} // namespace Asset

// data pointers

const char **g_defaultAssetName = reinterpret_cast<const char **>(0x823A40F8);

// function pointers

typedef int (*Com_sprintf_t)(char *dest, unsigned int size, const char *fmt, ...);
Com_sprintf_t Com_sprintf = reinterpret_cast<Com_sprintf_t>(0x821CCED8);

typedef void (*DB_SetXAssetName_t)(XAsset *asset, const char *name);
DB_SetXAssetName_t DB_SetXAssetName = reinterpret_cast<DB_SetXAssetName_t>(0x822B30D0);

typedef XAssetEntry *(*DB_LinkXAssetEntry_t)(XAssetEntry *newEntry, int allowOverride);
DB_LinkXAssetEntry_t DB_LinkXAssetEntry = reinterpret_cast<DB_LinkXAssetEntry_t>(0x8229FC50);

struct snapshotEntityNumbers_t;

typedef void (*SV_AddEntitiesVisibleFromPoint_t)(const float *org, int clientNum, snapshotEntityNumbers_t *eNums);
SV_AddEntitiesVisibleFromPoint_t SV_AddEntitiesVisibleFromPoint =
    reinterpret_cast<SV_AddEntitiesVisibleFromPoint_t>(0x821FB898);

Detour DB_LinkXAssetEntry_Detour;
XAssetEntry *DB_LinkXAssetEntry_Hook(XAssetEntry *newEntry, int allowOverride)
{
    XAsset xasset;
    xasset.type = newEntry->asset.type;
    xasset.header = newEntry->asset.header;

    if (mpsp::is_sp_map)
    {
        switch (newEntry->asset.type)
        {
        case ASSET_TYPE_MAP_ENTS:
        {
            Asset::MapEnts_::override_(newEntry->asset.header.mapEnts);
            break;
        }
        case ASSET_TYPE_RAWFILE:
        {
            Asset::RawFile_::override_(newEntry->asset.header.rawfile);
            break;
        }
        case ASSET_TYPE_GAMEWORLD_SP:
        {
            newEntry->asset.type = ASSET_TYPE_GAMEWORLD_MP;
            break;
        }
        // Hijack the reference asset ',' mechanism to avoid reaching asset limits.
        case ASSET_TYPE_WEAPON:
        {
            static const std::string weapon_default_reference_name =
                std::string(",") + g_defaultAssetName[ASSET_TYPE_WEAPON];
            DB_SetXAssetName(&xasset, weapon_default_reference_name.c_str());
            break;
        }
        }
    }

    return DB_LinkXAssetEntry_Detour.GetOriginal<DB_LinkXAssetEntry_t>()(newEntry, allowOverride);
}

Detour Com_sprintf_Detour;
int Com_sprintf_Hook(char *dest, unsigned int size, const char *fmt...)
{
    // Do original logic
    va_list va;
    va_start(va, fmt);
    int result = _vsnprintf(dest, size, fmt, va);
    va_end(va);

    if (size > 0)
        dest[size - 1] = '\0';

    // Rewrites

    // [mpsp]
    // Com_GetBspFilename has been inlined
    const void *returnAddress = _ReturnAddress();
    if (returnAddress == (void *)0x82203164    // SV_SpawnServer
        || returnAddress == (void *)0x822E7044 // CL_InitCGame
        || returnAddress == (void *)0x822F9EE8 // CG_ParseServerInfo
        // || returnAddress == (void *)0x823283C8 //
        // || returnAddress == (void *)0x823283E8 //

    )
    {
        // "maps/mp/%s.d3dbsp"
        const char *mapname = dest + 8; // skip "maps/mp/"
        const bool isMp = (strncmp(mapname, "mp_", 3) == 0);

        if (!isMp)
        {

            char *dst = dest + 5;       // after "maps/"
            const char *src = dest + 8; // after "maps/mp/"
            memmove(dst, src, strlen(src) + 1);

            DbgPrint("Rewrote BSP path to %s\n", dest);

            mpsp::is_sp_map = true;
        }
    }

    // [mpsp]
    if (returnAddress == (void *)0x82265820) // GScr_LoadLevelScript
    {
        // "maps/mp/%s"
        const char *mapname = dest + 8; // skip "maps/mp/"
        const bool isMp = (strncmp(mapname, "mp_", 3) == 0);
        if (!isMp)
        {
            // Rewrite to "maps/%s"
            char *dst = dest + 5;       // after "maps/"
            const char *src = dest + 8; // after "maps/mp/"
            memmove(dst, src, strlen(src) + 1);
        }

        DbgPrint("Com_sprintf_Hook <- GScr_LoadLevelScript\n");
        DbgPrint("%s\n", dest);
    }

    return result;
}

Detour SV_AddEntitiesVisibleFromPoint_Detour;
void SV_AddEntitiesVisibleFromPoint_Hook(const float *org, int clientNum, snapshotEntityNumbers_t *eNums)
{
    if (mpsp::is_sp_map)
    {
        return;
    }

    SV_AddEntitiesVisibleFromPoint_Detour.GetOriginal<SV_AddEntitiesVisibleFromPoint_t>()(org, clientNum, eNums);
}

mpsp::mpsp()
{

    Com_sprintf_Detour = Detour(Com_sprintf, Com_sprintf_Hook);
    Com_sprintf_Detour.Install();

    DB_LinkXAssetEntry_Detour = Detour(DB_LinkXAssetEntry, DB_LinkXAssetEntry_Hook);
    DB_LinkXAssetEntry_Detour.Install();

    SV_AddEntitiesVisibleFromPoint_Detour = Detour(SV_AddEntitiesVisibleFromPoint, SV_AddEntitiesVisibleFromPoint_Hook);
    SV_AddEntitiesVisibleFromPoint_Detour.Install();
}

mpsp::~mpsp()
{

    Com_sprintf_Detour.Remove();

    DB_LinkXAssetEntry_Detour.Remove();

    SV_AddEntitiesVisibleFromPoint_Detour.Remove();
}

} // namespace mp
} // namespace iw3
