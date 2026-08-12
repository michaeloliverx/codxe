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

std::map<std::string, ScriptFile *> g_loaded_scripts;

const unsigned int GSC_BYTECODE_ARENA_SIZE = 256 * 1024;
unsigned __int8 *g_gsc_bytecode_arena = nullptr;
unsigned int g_gsc_bytecode_arena_used = 0;

unsigned __int8 *AllocateGSCBytecode(unsigned int size)
{
    if (!g_gsc_bytecode_arena)
    {
        const bool is_xenia = xbox::GetEnvironment() == xbox::ENVIRONMENT_XENIA;
        g_gsc_bytecode_arena = PMem_AllocFromSource_NoDebug(GSC_BYTECODE_ARENA_SIZE, 4,
                                                            // 0 crashes on hardware, 2 crashes on Xenia.
                                                            is_xenia ? 0 : 2, PMEM_SOURCE_SCRIPT);
    }

    const unsigned int offset = (g_gsc_bytecode_arena_used + 3) & ~3u;
    if (offset > GSC_BYTECODE_ARENA_SIZE || size > GSC_BYTECODE_ARENA_SIZE - offset)
    {
        DbgPrint("[codxe][IW5][GSCLoader] bytecode arena exhausted: requested=%u used=%u capacity=%u\n", size,
                 g_gsc_bytecode_arena_used, GSC_BYTECODE_ARENA_SIZE);
        return nullptr;
    }

    unsigned __int8 *bytecode = g_gsc_bytecode_arena + offset;
    g_gsc_bytecode_arena_used = offset + size;
    return bytecode;
}

bool ContainsScript(const std::string &name)
{
    return g_loaded_scripts.find(name) != g_loaded_scripts.end();
}

void ResetLoadedScripts(bool freeScripts)
{
    if (!freeScripts)
        return;

    for (auto it = g_loaded_scripts.begin(); it != g_loaded_scripts.end(); ++it)
    {
        ScriptFile *scriptfile = it->second;
        free(const_cast<char *>(scriptfile->buffer));
        free(scriptfile);
    }

    g_loaded_scripts.clear();
    g_gsc_bytecode_arena = nullptr;
    g_gsc_bytecode_arena_used = 0;
}

/**
 * GSC Tool binary file format.
 * https://github.com/xensik/gsc-tool?tab=readme-ov-file#file-format
 */
struct GSCBin
{
    char magic[4];                 // Magic identifier "GSC\0"
    uint32_t compressedLen;        // Length of the compressed buffer
    uint32_t len;                  // Original uncompressed length
    uint32_t bytecodeLen;          // Length of the bytecode
    std::vector<uint8_t> buffer;   // Compressed buffer data
    std::vector<uint8_t> bytecode; // Bytecode data
};

bool LoadGSCBin(const char *filename, GSCBin &gsc)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        DbgPrint("Error: Unable to open file: %s\n", filename);
        return false;
    }

    // Read the fixed-size header fields
    file.read(reinterpret_cast<char *>(gsc.magic), sizeof(gsc.magic));
    file.read(reinterpret_cast<char *>(&gsc.compressedLen), sizeof(gsc.compressedLen));
    file.read(reinterpret_cast<char *>(&gsc.len), sizeof(gsc.len));
    file.read(reinterpret_cast<char *>(&gsc.bytecodeLen), sizeof(gsc.bytecodeLen));

    // Convert from little-endian to big-endian
    gsc.compressedLen = endian::ByteSwap(gsc.compressedLen);
    gsc.len = endian::ByteSwap(gsc.len);
    gsc.bytecodeLen = endian::ByteSwap(gsc.bytecodeLen);

    // Check if the magic identifier is correct
    if (strncmp(gsc.magic, "GSC\0", 4) != 0)
    {
        DbgPrint("Error: Invalid GSC magic identifier.\n");
        return false;
    }

    // Allocate memory for buffer and bytecode
    gsc.buffer.resize(gsc.compressedLen);
    gsc.bytecode.resize(gsc.bytecodeLen);

    // Read the compressed buffer and bytecode data
    file.read(reinterpret_cast<char *>(gsc.buffer.data()), gsc.compressedLen);
    file.read(reinterpret_cast<char *>(gsc.bytecode.data()), gsc.bytecodeLen);

    // Close the file
    file.close();

    return true;
}

Detour DB_FindXAssetHeader_Detour;

XAssetHeader *DB_FindXAssetHeader_Hook(XAssetType type, const char *name, int allowCreateDefault)
{
    if (type == ASSET_TYPE_SCRIPTFILE)
    {
        auto loadedScript = g_loaded_scripts.find(name);
        if (loadedScript != g_loaded_scripts.end())
            return reinterpret_cast<XAssetHeader *>(loadedScript->second);

        std::string modBasePath = Config::GetModBasePath();
        std::string overridePath = modBasePath + "\\" + name + ".gscbin";
        std::replace(overridePath.begin(), overridePath.end(), '/', '\\');

        if (!modBasePath.empty())
        {
            GSCBin gscbin;
            if (!LoadGSCBin(overridePath.c_str(), gscbin))
            {
                DbgPrint("Failed to load GSC file.\n");
            }
            else
            {
                // ProcessScript treats these as persistent writable data. Keeping them on the heap avoids
                // consuming a 64 KiB script page for each small allocation.
                ScriptFile *scriptfile = static_cast<ScriptFile *>(malloc(sizeof(ScriptFile)));
                memset(scriptfile, 0, sizeof(ScriptFile));

                scriptfile->name = name;
                scriptfile->compressedLen = gscbin.compressedLen;
                scriptfile->len = gscbin.len;
                scriptfile->bytecodeLen = gscbin.bytecodeLen;

                char *buffer = static_cast<char *>(malloc(gscbin.buffer.size()));
                memcpy(buffer, gscbin.buffer.data(), gscbin.buffer.size());
                scriptfile->buffer = buffer;

                unsigned __int8 *bytecode = AllocateGSCBytecode(gscbin.bytecodeLen);
                if (!bytecode)
                {
                    free(buffer);
                    free(scriptfile);
                    return DB_FindXAssetHeader_Detour.GetOriginal<DB_FindXAssetHeader_t>()(type, name,
                                                                                           allowCreateDefault);
                }
                memcpy(bytecode, gscbin.bytecode.data(), gscbin.bytecode.size());
                scriptfile->bytecode = bytecode;

                g_loaded_scripts[name] = scriptfile;

                return (XAssetHeader *)scriptfile;
            }
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
    if (type == ASSET_TYPE_SCRIPTFILE && ContainsScript(name))
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

    Events::OnVMShutdown(ResetLoadedScripts);
}

IW5_MP_Plugin::~IW5_MP_Plugin()
{
    ResetLoadedScripts(true);
    DB_FindXAssetHeader_Detour.Remove();
    DB_IsXAssetDefault_Detour.Remove();
}

} // namespace mp
} // namespace iw5
