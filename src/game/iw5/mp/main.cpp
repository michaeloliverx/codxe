#include "pch.h"
#include "common/endian.h"
#include "branding.h"
#include "main.h"
#include "patches.h"
#include "pm.h"
#include "gsc.h"
#include "bots.h"
#include "events.h"

namespace iw5
{
namespace mp
{
namespace
{
const unsigned int GSC_ARENA_SIZE = 256 * 1024;
const unsigned int MAX_LOADED_SCRIPTS = 128;

struct GSCBinHeader
{
    char magic[4];
    uint32_t compressedLen;
    uint32_t len;
    uint32_t bytecodeLen;
};

static_assert(sizeof(GSCBinHeader) == 16, "GSCBinHeader size mismatch");

ScriptFile *g_loaded_scripts[MAX_LOADED_SCRIPTS];
unsigned int g_loaded_script_count = 0;
unsigned __int8 *g_gsc_arena = nullptr;
unsigned int g_gsc_arena_used = 0;

void NormalizePath(char *path)
{
    if (path == nullptr)
        return;

    for (char *cursor = path; *cursor != '\0'; ++cursor)
    {
        if (*cursor == '/')
            *cursor = '\\';
    }
}

bool BuildScriptPath(char *path, size_t pathSize, const char *name)
{
    const char *modBasePath = Config::GetModBasePath();
    if (path == nullptr || pathSize == 0 || modBasePath == nullptr || modBasePath[0] == '\0' || name == nullptr ||
        name[0] == '\0')
    {
        return false;
    }

    const int written = _snprintf_s(path, pathSize, _TRUNCATE, "%s\\%s.gscbin", modBasePath, name);
    path[pathSize - 1] = '\0';
    if (written < 0 || static_cast<size_t>(written) >= pathSize)
        return false;

    NormalizePath(path);
    return true;
}

void *AllocateGSCData(unsigned int size, unsigned int alignment)
{
    if (alignment == 0 || (alignment & (alignment - 1)) != 0)
        return nullptr;

    if (g_gsc_arena == nullptr)
    {
        const bool isXenia = xbox::GetEnvironment() == xbox::ENVIRONMENT_XENIA;
        g_gsc_arena = PMem_AllocFromSource_NoDebug(GSC_ARENA_SIZE, 4,
                                                   // 0 crashes on hardware, 2 crashes on Xenia.
                                                   isXenia ? 0 : 2, PMEM_SOURCE_SCRIPT);
        if (g_gsc_arena == nullptr)
            return nullptr;
    }

    const unsigned int offset = (g_gsc_arena_used + alignment - 1) & ~(alignment - 1);
    if (offset < g_gsc_arena_used || offset > GSC_ARENA_SIZE || size > GSC_ARENA_SIZE - offset)
    {
        DbgPrint("[codxe][IW5][GSCLoader] arena exhausted: requested=%u used=%u capacity=%u\n", size, g_gsc_arena_used,
                 GSC_ARENA_SIZE);
        return nullptr;
    }

    void *data = g_gsc_arena + offset;
    g_gsc_arena_used = offset + size;
    return data;
}

ScriptFile *FindLoadedScript(const char *name)
{
    if (name == nullptr)
        return nullptr;

    for (unsigned int i = 0; i < g_loaded_script_count; ++i)
    {
        ScriptFile *scriptFile = g_loaded_scripts[i];
        if (scriptFile != nullptr && scriptFile->name != nullptr && strcmp(scriptFile->name, name) == 0)
            return scriptFile;
    }

    return nullptr;
}

void ResetLoadedScripts(bool freeScripts)
{
    if (!freeScripts)
        return;

    ZeroMemory(g_loaded_scripts, sizeof(g_loaded_scripts));
    g_loaded_script_count = 0;
    g_gsc_arena = nullptr;
    g_gsc_arena_used = 0;
}

bool ReadExact(HANDLE file, void *buffer, DWORD size)
{
    if (size == 0)
        return true;

    DWORD bytesRead = 0;
    return ReadFile(file, buffer, size, &bytesRead, nullptr) && bytesRead == size;
}

/**
 * GSC Tool binary file format.
 * https://github.com/xensik/gsc-tool?tab=readme-ov-file#file-format
 */
ScriptFile *LoadGSCBin(const char *path, const char *name)
{
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return nullptr;

    const DWORD fileSize = GetFileSize(file, nullptr);
    GSCBinHeader header;
    if (fileSize == INVALID_FILE_SIZE || fileSize < sizeof(header) || !ReadExact(file, &header, sizeof(header)))
    {
        CloseHandle(file);
        return nullptr;
    }

    header.compressedLen = endian::ByteSwap(header.compressedLen);
    header.len = endian::ByteSwap(header.len);
    header.bytecodeLen = endian::ByteSwap(header.bytecodeLen);

    const DWORD payloadSize = fileSize - sizeof(header);
    if (strncmp(header.magic, "GSC\0", sizeof(header.magic)) != 0 || header.compressedLen > payloadSize ||
        header.bytecodeLen != payloadSize - header.compressedLen)
    {
        DbgPrint("[codxe][IW5][GSCLoader] invalid GSC file: %s\n", path);
        CloseHandle(file);
        return nullptr;
    }

    if (g_loaded_script_count >= MAX_LOADED_SCRIPTS)
    {
        DbgPrint("[codxe][IW5][GSCLoader] registry exhausted: capacity=%u\n", MAX_LOADED_SCRIPTS);
        CloseHandle(file);
        return nullptr;
    }

    const unsigned int arenaMark = g_gsc_arena_used;
    ScriptFile *scriptFile = static_cast<ScriptFile *>(AllocateGSCData(sizeof(ScriptFile), 4));
    const unsigned int nameSize = static_cast<unsigned int>(strlen(name) + 1);
    char *storedName = static_cast<char *>(AllocateGSCData(nameSize, 1));
    char *buffer = static_cast<char *>(AllocateGSCData(header.compressedLen, 1));
    unsigned __int8 *bytecode = static_cast<unsigned __int8 *>(AllocateGSCData(header.bytecodeLen, 4));

    if (scriptFile == nullptr || storedName == nullptr || buffer == nullptr || bytecode == nullptr ||
        !ReadExact(file, buffer, header.compressedLen) || !ReadExact(file, bytecode, header.bytecodeLen))
    {
        g_gsc_arena_used = arenaMark;
        CloseHandle(file);
        return nullptr;
    }

    CloseHandle(file);

    memcpy(storedName, name, nameSize);
    scriptFile->name = storedName;
    scriptFile->compressedLen = header.compressedLen;
    scriptFile->len = header.len;
    scriptFile->bytecodeLen = header.bytecodeLen;
    scriptFile->buffer = buffer;
    scriptFile->bytecode = bytecode;

    g_loaded_scripts[g_loaded_script_count++] = scriptFile;
    DbgPrint("[codxe][IW5][GSCLoader] loaded override: %s\n", path);
    return scriptFile;
}
} // namespace

void IW5_MP_Plugin::OnVMShutdown(bool freeScripts)
{
    ResetLoadedScripts(freeScripts);
}

Detour DB_FindXAssetHeader_Detour;

XAssetHeader *DB_FindXAssetHeader_Hook(XAssetType type, const char *name, int allowCreateDefault)
{
    if (type == ASSET_TYPE_SCRIPTFILE)
    {
        ScriptFile *loadedScript = FindLoadedScript(name);
        if (loadedScript != nullptr)
            return reinterpret_cast<XAssetHeader *>(loadedScript);

        char overridePath[MAX_PATH];
        if (BuildScriptPath(overridePath, sizeof(overridePath), name))
        {
            ScriptFile *scriptFile = LoadGSCBin(overridePath, name);
            if (scriptFile != nullptr)
                return reinterpret_cast<XAssetHeader *>(scriptFile);
        }
    }

    XAssetHeader *header =
        DB_FindXAssetHeader_Detour.GetOriginal<DB_FindXAssetHeader_t>()(type, name, allowCreateDefault);

    return header;
}

Detour DB_IsXAssetDefault_Detour;

bool DB_IsXAssetDefault_Hook(XAssetType type, const char *name)
{
    // Custom ScriptFile must return 0 to be loaded properly
    if (type == ASSET_TYPE_SCRIPTFILE && FindLoadedScript(name) != nullptr)
        return false;

    return DB_IsXAssetDefault_Detour.GetOriginal<DB_IsXAssetDefault_t>()(type, name);
}

IW5_MP_Plugin::IW5_MP_Plugin()
{
    RegisterModule(new Config());
    RegisterModule(new Events());
    RegisterModule(new Branding());
    RegisterModule(new patches());
    RegisterModule(new PlayerMovement());
    RegisterModule(new GSC());
    RegisterModule(new Bots());

    DB_FindXAssetHeader_Detour = Detour(DB_FindXAssetHeader, DB_FindXAssetHeader_Hook);
    DB_FindXAssetHeader_Detour.Install();

    DB_IsXAssetDefault_Detour = Detour(DB_IsXAssetDefault, DB_IsXAssetDefault_Hook);
    DB_IsXAssetDefault_Detour.Install();
}

IW5_MP_Plugin::~IW5_MP_Plugin()
{
    ResetLoadedScripts(true);
    DB_FindXAssetHeader_Detour.Remove();
    DB_IsXAssetDefault_Detour.Remove();
}

} // namespace mp
} // namespace iw5
