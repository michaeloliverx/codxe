

#include "pch.h"
#include "mpsp.h"
#include "unordered_map"

namespace iw4
{
namespace mp_tu6
{
bool mpsp::is_sp_map = false;

bool is_mp_fastfile(const char *name)
{
    if (!name)
        return false;

    const size_t len = std::strlen(name);

    // starts with "mp_"
    if (len >= 3 && std::strncmp(name, "mp_", 3) == 0)
        return true;

    // ends with "_mp"
    if (len >= 3 && std::strcmp(name + len - 3, "_mp") == 0)
        return true;

    return false;
}

struct internal_state;

struct Sys_File
{
    void *handle;
    int startOffset;
};

struct DBFile
{
    Sys_File handle;
    char name[64];
};

struct XBlock
{
    unsigned __int8 *data;
    unsigned int size;
};

struct XZoneMemory
{
    XBlock blocks[6];
};

struct XZone
{
    DBFile file;
    int flags;
    int allocType;
    XZoneMemory mem;
};

struct _OVERLAPPED
{
    unsigned int Internal;
    unsigned int InternalHigh;
    unsigned int Offset;
    unsigned int OffsetHigh;
    void *hEvent;
};

/* 9458 */
struct z_stream_s
{
    unsigned __int8 *next_in;
    unsigned int avail_in;
    unsigned int total_in;
    unsigned __int8 *next_out;
    unsigned int avail_out;
    unsigned int total_out;
    char *msg;
    internal_state *state;
    unsigned __int8 *(__fastcall *zalloc)(unsigned __int8 *, unsigned int, unsigned int);
    void(__fastcall *zfree)(unsigned __int8 *, unsigned __int8 *);
    unsigned __int8 *opaque;
    int data_type;
};

/* 10286 */
struct DB_LoadData
{
    DBFile *file;
    int outstandingRead;
    unsigned __int8 *fileBuffer;
    unsigned int readSize;
    unsigned int completedReadSize;
    unsigned int offset;
    unsigned __int8 *start_in;
    _OVERLAPPED overlapped;
    unsigned int readError;
    z_stream_s stream;
    unsigned int lookaheadReadSize;
    unsigned int lookaheadOffset;
    unsigned int lookaheadClearAvailIn;
};

typedef int (*DB_GetXAssetSizeHandlerFunc)();
DB_GetXAssetSizeHandlerFunc *DB_GetXAssetSizeHandler = reinterpret_cast<DB_GetXAssetSizeHandlerFunc *>(0x82442490);
void **DB_XAssetPool = reinterpret_cast<void **>(0x82442828);
const char **g_assetNames = reinterpret_cast<const char **>(0x82442298);
int *g_poolSize = reinterpret_cast<int *>(0x82442588);
const DB_LoadData *g_load = reinterpret_cast<DB_LoadData *>(0x82678600);
const unsigned int *g_zoneIndex = reinterpret_cast<const unsigned int *>(0x827ADAE4);
const XZone *g_zones = reinterpret_cast<XZone *>(0x829D8048);
GameWorldMp *gameWorldMp = reinterpret_cast<GameWorldMp *>(0x82DFD010);

typedef int (*Com_sprintf_t)(char *dest, unsigned int size, const char *fmt, ...);
Com_sprintf_t Com_sprintf = reinterpret_cast<Com_sprintf_t>(0x82315F20);

typedef XAssetHeader *(*DB_FindXAssetHeader_t)(XAssetType type, const char *name);
DB_FindXAssetHeader_t DB_FindXAssetHeader = reinterpret_cast<DB_FindXAssetHeader_t>(0x821E25B0);

typedef const char *(*DB_GetXAssetName_t)(const XAsset *asset);
DB_GetXAssetName_t DB_GetXAssetName = reinterpret_cast<DB_GetXAssetName_t>(0x821AEFD8);

typedef void (*DB_SetXAssetName_t)(XAsset *asset, const char *name);
DB_SetXAssetName_t DB_SetXAssetName = reinterpret_cast<DB_SetXAssetName_t>(0x821AEFF8);

typedef void (*DB_LoadXFile_t)(XZoneMemory *zoneMem, DBFile *file);
DB_LoadXFile_t DB_LoadXFile = reinterpret_cast<DB_LoadXFile_t>(0x821AFCB8);

typedef XAssetEntryPoolEntry *(*DB_LinkXAssetEntry1_t)(XAssetType type, XAssetHeader *header);
// DB_LinkXAssetEntry1_t DB_LinkXAssetEntry1 = reinterpret_cast<DB_LinkXAssetEntry1_t>(0x821DE528);

Detour CL_ConsolePrint_Detour;
void CL_ConsolePrint_Hook(int localClientNum, int channel, const char *txt, unsigned int duration,
                          unsigned int pixelWidth, int flags)
{
    DbgPrint("[CL_ConsolePrint] %s", txt);

    CL_ConsolePrint_Detour.GetOriginal<CL_ConsolePrint_t>()(localClientNum, channel, txt, duration, pixelWidth, flags);
}

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

std::string get_dump_dir()
{
    return "game:\\_dump";
}

std::string get_load_dir()
{
    return "game:\\_codxe\\internal";
}
namespace MapEnts_
{

std::string mapents_buffer;

void dump(MapEnts *asset)
{
    std::string buffer;

    if (!asset || !asset->name || asset->name[0] == '\0')
    {
        return;
    }

    // Don't attempt to dump referenced assets
    if (asset->name[0] == ',')
    {
        return;
    }

    DbgPrint("Dumping mapents asset %s from fastfile %s\n", asset->name, g_load->file->name);

    const std::string filename = Asset::get_dump_dir() + "\\" + asset->name + ".ents";
    FS::WriteFile(filename, asset->entityString, asset->numEntityChars - 1);
}

void override_(MapEnts *asset)
{
    // _load\maps\jeepride.d3dbsp.ents
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

    DbgPrint("Overiding mapents asset %s from fastfile %s\n", asset->name, g_load->file->name);

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
    /*
        NOTE:
        Some RawFile assets in fastfiles are compressed, but the engine will
        accept an uncompressed override. Setting:

            asset->compressedLen = 0;

        forces the loader to skip decompression and treat our buffer as plain
        uncompressed data. After that, setting asset->len and asset->buffer
        is enough for the engine to load the override correctly.
    */

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

    // NOTE: It seems that the engine will handle us giving back uncompressed strings (tested at least for .gsc files!)

    DbgPrint("Overriding rawfile asset %s from fastfile %s\n", asset->name, g_load->file->name);

    auto itr = rawfile_buffers.find(asset->name);
    if (itr != rawfile_buffers.end())
    {
        rawfile_buffers.erase(itr);
    }
    rawfile_buffers[asset->name] = make_unique<std::string>();
    itr = rawfile_buffers.find(asset->name);
    auto rawfile_buffer = itr->second.get();

    rawfile_buffer->assign(buffer);

    asset->compressedLen = 0; // Force the engine to treat it as an uncompressed buffer!
    asset->len = rawfile_buffer->length();
    asset->buffer = rawfile_buffer->data();
}

} // namespace RawFile_

} // namespace Asset

Detour DB_LinkXAssetEntry1_Detour;
XAssetEntryPoolEntry *DB_LinkXAssetEntry1_Hook(XAssetType type, XAssetHeader *header)
{
    XAsset xasset;
    xasset.type = type;
    xasset.header = *header;

    const char *asset_name = DB_GetXAssetName(&xasset);

    // Before linking
    switch (type)
    {
    case ASSET_TYPE_MAP_ENTS:
        Asset::MapEnts_::dump(header->mapEnts);
        Asset::MapEnts_::override_(header->mapEnts);
        break;
    case ASSET_TYPE_RAWFILE:
        Asset::RawFile_::override_(header->rawfile);
        break;
    }

    // Hijacks the referenced asset mechanism so that DB_AllocXAssetEntry
    // is never called thus never increasing the asset pool for those assets.
    // Make the assumption that these assets will not be used.
    if (!is_mp_fastfile(g_load->file->name))
    {
        switch (type)
        {
        case ASSET_TYPE_SOUND:
            DB_SetXAssetName(&xasset, ",null");
            break;
        case ASSET_TYPE_LOADED_SOUND:
            DB_SetXAssetName(&xasset, ",null");
            break;
        case ASSET_TYPE_FX:
            DB_SetXAssetName(&xasset, ",misc/missing_fx");
            break;
        case ASSET_TYPE_XMODEL:
            if (strncmp(asset_name, "viewmodel_", 10) == 0 && strncmp(asset_name, "viewmodel_base_viewhands", 24) != 0)
            {
                DbgPrint("referenced asset hack viewmodel_base_viewhands\n");
                DB_SetXAssetName(&xasset, ",viewmodel_base_viewhands");
            }
            else if (strncmp(asset_name, "weapon_", 7) == 0)
            {
                DbgPrint("referenced asset hack ASSET_TYPE_XMODEL weapon_\n");
                DB_SetXAssetName(&xasset, ",void");
            }
            break;
        case ASSET_TYPE_MATERIAL:
            if (strncmp(asset_name, "mc/mtl_weapon_", 14) == 0)
            {
                DbgPrint("referenced asset hack ASSET_TYPE_MATERIAL mc/mtl_weapon_\n");
                DB_SetXAssetName(&xasset, ",$default");
            }
            if (strncmp(asset_name, "hud_", 4) == 0)
            {
                DbgPrint("referenced asset hack ASSET_TYPE_MATERIAL hud_\n");
                DB_SetXAssetName(&xasset, ",$default");
            }
            break;
        case ASSET_TYPE_GAMEWORLD_SP:
            type = ASSET_TYPE_GAMEWORLD_MP;
            header->gameWorldMp->g_glassData = header->gameWorldSp->g_glassData;
            break;
        case ASSET_TYPE_LOCALIZE_ENTRY:
            DB_SetXAssetName(&xasset, ",CGAME_UNKNOWN");
        }
    }

    XAssetEntryPoolEntry *entry = DB_LinkXAssetEntry1_Detour.GetOriginal<DB_LinkXAssetEntry1_t>()(type, header);

    return entry;
}

void DB_ReallocXAssetPool(XAssetType type, unsigned int newSize)
{
    void *pool_entry = malloc(newSize * DB_GetXAssetSizeHandler[type]());
    DB_XAssetPool[type] = pool_entry;
    g_poolSize[type] = newSize;
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
    const void *returnaddr = _ReturnAddress();
    if (returnaddr == (void *)0x822F6AD8) // Com_GetBspFilename
    {
        // Format is always "maps/mp/<name>.d3dbsp"
        const char *mapname = dest + 8; // skip "maps/mp/"
        const bool isMp = (strncmp(mapname, "mp_", 3) == 0);

        if (!isMp)
        {
            // Rewrite to "maps/<name>.d3dbsp"
            char *dst = dest + 5;       // after "maps/"
            const char *src = dest + 8; // after "maps/mp/"
            memmove(dst, src, strlen(src) + 1);
        }
    }

    // [mpsp]
    if (returnaddr == (void *)0x8224EDA0) // GScr_LoadLevelScript
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
    }

    return result;
}

void Patch_ImageCache_OOM()
{
    // ImageCache_AllocMemory
    // 0x823DF628: bne cr6, loc_823DF6CC  ->  b loc_823DF6CC
    // Original: 0x409E00A4
    // Patched:  0x480000A4    (unconditional relative branch +0xA4)
    *(volatile uint32_t *)0x823DF628 = 0x480000A4;
}

mpsp::mpsp()
{
    Patch_ImageCache_OOM();

// Debug-only: intercept internal console printing to also forward output to stdout
#ifndef NDEBUG
    CL_ConsolePrint_Detour = Detour(CL_ConsolePrint, CL_ConsolePrint_Hook);
    CL_ConsolePrint_Detour.Install();
#endif

    // Modify some assets before linking
    DB_LinkXAssetEntry1_Detour = Detour(DB_LinkXAssetEntry1, DB_LinkXAssetEntry1_Hook);
    DB_LinkXAssetEntry1_Detour.Install();

    // Rewrite some strings on the fly
    Com_sprintf_Detour = Detour(Com_sprintf, Com_sprintf_Hook);
    Com_sprintf_Detour.Install();
}

mpsp::~mpsp()
{

#ifndef NDEBUG
    CL_ConsolePrint_Detour.Remove();
#endif

    DB_LinkXAssetEntry1_Detour.Remove();

    Com_sprintf_Detour.Remove();
}

} // namespace mp_tu6
} // namespace iw4
