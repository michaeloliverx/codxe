#include "pch.h"
#include "components/command.h"
#include "components/events.h"
#include "components/gsc.h"
#include "components/scr_parser.h"

namespace iw3
{
namespace sp
{

Detour Load_MapEntsPtr_Detour;

void Load_MapEntsPtr_Hook()
{
    DbgPrint("Load_MapEntsPtr_Hook\n");

    // TODO: write comment what this is ***
    // Get pointer to pointer stored at 0x82475914
    MapEnts **varMapEntsPtr = *(MapEnts ***)0x825875D8;

    Load_MapEntsPtr_Detour.GetOriginal<decltype(&Load_MapEntsPtr_Hook)>()();

    // Validate pointer before dereferencing
    if (varMapEntsPtr && *varMapEntsPtr)
    {
        MapEnts *mapEnts = *varMapEntsPtr;

        // Dump stock map ents if enabled (numEntityChars includes the null terminator, which is not written)
        if (Config::dump_map_ents && mapEnts->numEntityChars > 0)
        {
            const std::string dumpPath = map_ents::BuildPath(DUMP_DIR, mapEnts->name);
            if (!dumpPath.empty())
            {
                filesystem::WriteFileToDisk(dumpPath.c_str(), mapEnts->entityString, mapEnts->numEntityChars - 1);
                DbgPrint("Dumped map ents to: %s\n", dumpPath.c_str());
            }
        }

        // Load map ents from file
        // Path to check for existing entity file
        const std::string rawFilePath =
            map_ents::BuildPath(filesystem::JoinPath(Config::GetDataRoot(), "raw").c_str(), mapEnts->name);
        if (rawFilePath.empty())
        {
            return;
        }

        // If the file exists, replace entityString
        if (filesystem::FileExists(rawFilePath.c_str()))
        {
            DbgPrint("Found entity file: %s\n", rawFilePath.c_str());
            std::string new_entity_string = filesystem::ReadFileToString(rawFilePath);
            if (!new_entity_string.empty())
            {
                // Allocate new memory and copy the data
                size_t new_size = new_entity_string.size() + 1; // Include null terminator
                char *new_memory = static_cast<char *>(malloc(new_size));

                if (new_memory)
                {
                    memcpy(new_memory, new_entity_string.c_str(), new_size); // Copy with null terminator
                    mapEnts->entityString = new_memory;
                    DbgPrint("Replaced entityString from file: %s\n", rawFilePath.c_str());
                }
                else
                {
                    DbgPrint("Failed to allocate memory for entityString replacement.\n");
                }
            }
        }
    }
    else
    {
        DbgPrint("Hooked Load_MapEntsPtr: varMapEntsPtr is NULL or invalid.\n");
    }
}

void Cmd_Dumpraw_f()
{
    XAssetHeader files[2048];
    auto count = DB_GetAllXAssetOfType_FastFile(ASSET_TYPE_RAWFILE, files, 2048);

    for (int i = 0; i < count; i++)
    {
        auto rawfile = files[i].rawfile;
        const std::string dumpPath = filesystem::JoinPath(DUMP_DIR, rawfile->name);
        filesystem::WriteFileToDisk(dumpPath.c_str(), rawfile->buffer, rawfile->len);
    }
}

IW3_SP_Plugin::IW3_SP_Plugin()
{
    DbgPrint("IW3 SP: Plugin loaded\n");
    RegisterModule(new Config());
    RegisterModule(new Events());
    RegisterModule(new command());
    RegisterModule(new GSC());
    RegisterModule(new scr_parser());

    Load_MapEntsPtr_Detour = Detour(Load_MapEntsPtr, Load_MapEntsPtr_Hook);
    Load_MapEntsPtr_Detour.Install();

    command::add("dumpraw", Cmd_Dumpraw_f);
}

IW3_SP_Plugin::~IW3_SP_Plugin()
{
    Load_MapEntsPtr_Detour.Remove();
}

} // namespace sp
} // namespace iw3
