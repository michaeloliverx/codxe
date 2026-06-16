#include "pch.h"
#include "components/cg.h"
#include "components/cj_tas.h"
#include "components/clipmap.h"
#include "components/command.h"
#include "components/cmds.h"
#include "components/console.h"
#include "components/events.h"
#include "components/g_scr_main.h"
#include "components/gsc_client_fields.h"
#include "components/gsc_functions.h"
#include "components/gsc_hud_elem.h"
#include "components/gsc_methods.h"
#include "components/image_loader.h"
#include "components/mpsp.h"
#include "components/pm.h"
#include "components/scr_parser.h"
#include "components/stats.h"
#include "components/sv_bots.h"
#include "common/config.h"
#include "main.h"

namespace iw3
{
namespace mp
{
Detour Load_MapEntsPtr_Detour;

void Load_MapEntsPtr_Hook()
{
    // TODO: don't write null byte to file
    // and add null byte to entityString when reading from file

    DbgPrint("Load_MapEntsPtr_Hook\n");

    // TODO: write comment what this is ***
    // Get pointer to pointer stored at 0x82475914
    MapEnts **varMapEntsPtr = *(MapEnts ***)0x82475914;

    Load_MapEntsPtr_Detour.GetOriginal<decltype(&Load_MapEntsPtr_Hook)>()();

    // Validate pointer before dereferencing
    if (varMapEntsPtr && *varMapEntsPtr)
    {
        MapEnts *mapEnts = *varMapEntsPtr;

        // Write stock map ents to disk
        std::string file_path = DUMP_DIR;
        file_path += std::string("\\") + mapEnts->name;
        file_path += ".ents";                                        //  iw4x naming convention
        std::replace(file_path.begin(), file_path.end(), '/', '\\'); // Replace forward slashes with backslashes
        filesystem::write_file_to_disk(file_path.c_str(), mapEnts->entityString, mapEnts->numEntityChars);

        // Use new ConfigModule API
        // Load map ents from file
        // Path to check for existing entity file
        std::string raw_file_path = Config::GetModBasePath();

        raw_file_path += std::string("\\") + mapEnts->name;
        raw_file_path += ".ents";                                            // IW4x naming convention
        std::replace(raw_file_path.begin(), raw_file_path.end(), '/', '\\'); // Replace forward slashes with backslashes

        // If the file exists, replace entityString
        if (filesystem::file_exists(raw_file_path))
        {
            DbgPrint("Found entity file: %s\n", raw_file_path.c_str());
            std::string new_entity_string = filesystem::read_file_to_string(raw_file_path);
            if (!new_entity_string.empty())
            {
                // Allocate new memory and copy the data
                size_t new_size = new_entity_string.size() + 1; // Include null terminator
                char *new_memory = static_cast<char *>(malloc(new_size));

                if (new_memory)
                {
                    memcpy(new_memory, new_entity_string.c_str(), new_size); // Copy with null terminator

                    // Update the entityString pointer to point to the new memory
                    mapEnts->entityString = new_memory;

                    // // Update numEntityChars
                    // mapEnts->numEntityChars = static_cast<int>(new_entity_string.size());	// unnecessary

                    DbgPrint("Replaced entityString from file: %s\n", raw_file_path.c_str());
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

/**
 * Patch out the signature checks used during fastfile authentication.
 * Signature data must still be present in
 * the fastfile structure, but the values themselves may be zeroed.
 */
void DisableFastfileAuth()
{
    // DBX_AuthLoad_ValidateHash
    *(volatile uint32_t *)0x822B2994 = 0x60000000;
    *(volatile uint32_t *)0x822B2A34 = 0x60000000;
    *(volatile uint32_t *)0x822B2D2C = 0x60000000;

    // DBX_AuthLoad_ValidateSignature
    *(volatile uint32_t *)0x822B2D44 = 0x60000000;
}

IW3_MP_Plugin::IW3_MP_Plugin()
{
    DisableFastfileAuth();

    // default loc_warnings off to prevent console spam
    *(volatile uint8_t *)0x821FB069 = 0xE1;

    // Special modules need to be registered first
    RegisterModule(new Config());
    RegisterModule(new Events());
    RegisterModule(new command());

    RegisterModule(new cg());
    RegisterModule(new cj_tas());
    RegisterModule(new clipmap());
    RegisterModule(new cmds());
    RegisterModule(new console());
    RegisterModule(new g_scr_main());
    RegisterModule(new gsc_client_fields());
    RegisterModule(new gsc_functions());
    RegisterModule(new gsc_hud_elem());
    RegisterModule(new gsc_methods());
    RegisterModule(new image_loader());
    RegisterModule(new pm());
    RegisterModule(new mpsp());
    RegisterModule(new scr_parser());
    RegisterModule(new stats());
    RegisterModule(new sv_bots());

    Load_MapEntsPtr_Detour = Detour(Load_MapEntsPtr, Load_MapEntsPtr_Hook);
    Load_MapEntsPtr_Detour.Install();
}

IW3_MP_Plugin::~IW3_MP_Plugin()
{
    Load_MapEntsPtr_Detour.Remove();
}

} // namespace mp
} // namespace iw3
