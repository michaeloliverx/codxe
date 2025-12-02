#include "pch.h"
#include "branding.h"
#include "main.h"
#include "patches.h"
#include "pm.h"
#include "script.h"

namespace iw5
{
namespace mp
{

std::set<std::string> g_loaded_scripts;

bool ContainsScript(const std::string &name)
{
    return g_loaded_scripts.find(name) != g_loaded_scripts.end();
}

// Swap byte order for 32-bit integers
uint32_t SwapEndian(uint32_t value)
{
    return ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) | ((value & 0x00FF0000) >> 8) |
           ((value & 0xFF000000) >> 24);
}

void DumpScriptFileAsset(const ScriptFile *scriptfile)
{
    std::string filename = "game:\\_dump\\" + std::string(scriptfile->name) + ".gscbin";
    std::ofstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        DbgPrint("ERROR: Failed to open output file for writing\n");
        return;
    }
    // Write the custom header
    const char header[] = "GSC\0";
    file.write(header, sizeof(header) - 1); // Exclude the null terminator

    // Write compressedLen in little-endian
    uint32_t compressedLenLE = SwapEndian(static_cast<uint32_t>(scriptfile->compressedLen));
    file.write(reinterpret_cast<const char *>(&compressedLenLE), sizeof(compressedLenLE));

    // Write len in little-endian
    uint32_t lenLE = SwapEndian(static_cast<uint32_t>(scriptfile->len));
    file.write(reinterpret_cast<const char *>(&lenLE), sizeof(lenLE));

    // Write bytecodeLen in little-endian
    uint32_t bytecodeLenLE = SwapEndian(static_cast<uint32_t>(scriptfile->bytecodeLen));
    file.write(reinterpret_cast<const char *>(&bytecodeLenLE), sizeof(bytecodeLenLE));

    // Write buffer content as byte array[compressedLen]
    if (scriptfile->buffer && scriptfile->compressedLen > 0)
    {
        file.write(scriptfile->buffer, scriptfile->compressedLen);
    }
    else
    {
        // Write zero bytes if no buffer or invalid length
        std::vector<char> emptyBuffer(scriptfile->compressedLen, 0);
        file.write(emptyBuffer.data(), emptyBuffer.size());
    }

    // Write bytecode as byte array[bytecodeLen]
    if (scriptfile->bytecode && scriptfile->bytecodeLen > 0)
    {
        file.write(reinterpret_cast<const char *>(scriptfile->bytecode), scriptfile->bytecodeLen);
    }
    else
    {
        // Write zero bytes if no bytecode or invalid length
        std::vector<char> emptyBytecode(scriptfile->bytecodeLen, 0);
        file.write(emptyBytecode.data(), emptyBytecode.size());
    }

    file.close();

    DbgPrint("INFO: Script file binary dumped successfully.\n");
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
    gsc.compressedLen = _byteswap_ulong(gsc.compressedLen);
    gsc.len = _byteswap_ulong(gsc.len);
    gsc.bytecodeLen = _byteswap_ulong(gsc.bytecodeLen);

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
        Config config;
        LoadConfigFromFile(CONFIG_PATH, config);
        std::string modBasePath = config.GetModBasePath();
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
                // Create a new
                ScriptFile *scriptfile =
                    (ScriptFile *)PMem_AllocFromSource_NoDebug(sizeof(ScriptFile), 4, 0, PMEM_SOURCE_SCRIPT);
                memset(scriptfile, 0, sizeof(ScriptFile));

                scriptfile->name = name;
                scriptfile->compressedLen = gscbin.compressedLen;
                scriptfile->len = gscbin.len;
                scriptfile->bytecodeLen = gscbin.bytecodeLen;

                char *buffer = (char *)PMem_AllocFromSource_NoDebug(gscbin.buffer.size(), 4, 0, PMEM_SOURCE_SCRIPT);
                memcpy(buffer, gscbin.buffer.data(), gscbin.buffer.size());
                scriptfile->buffer = buffer;

                unsigned __int8 *bytecode = PMem_AllocFromSource_NoDebug(gscbin.bytecode.size(), 4,
                                                                         // 0 Crashes on hardware, 2 crashes on Xenia
                                                                         // Don't know why, but this works around it
                                                                         xbox::IsXenia() ? 0 : 2, PMEM_SOURCE_SCRIPT);
                memcpy(bytecode, gscbin.bytecode.data(), gscbin.bytecode.size());
                scriptfile->bytecode = bytecode;

                g_loaded_scripts.insert(name);

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
    DbgPrint("IW5 MP Plugin initialized\n");

    RegisterModule(new Branding());
    RegisterModule(new patches());
    RegisterModule(new PlayerMovement());
    RegisterModule(new Script());

    DB_FindXAssetHeader_Detour = Detour(DB_FindXAssetHeader, DB_FindXAssetHeader_Hook);
    DB_FindXAssetHeader_Detour.Install();

    DB_IsXAssetDefault_Detour = Detour(DB_IsXAssetDefault, DB_IsXAssetDefault_Hook);
    DB_IsXAssetDefault_Detour.Install();
}

IW5_MP_Plugin::~IW5_MP_Plugin()
{
    DbgPrint("IW5 MP Plugin shutting down\n");
    DB_FindXAssetHeader_Detour.Remove();
    DB_IsXAssetDefault_Detour.Remove();
}

bool IW5_MP_Plugin::ShouldLoad()
{
    return (strncmp((char *)0x82001458, "multiplayer", 11) == 0);
}
} // namespace mp
} // namespace iw5
