#pragma once

#include "games/iw3_mp/structs.h"

#include <cstdint>

namespace codxe
{
namespace games
{
namespace iw3_mp
{

typedef void (*Com_InitDvars_t)();
typedef void (*Scr_ShutdownSystem_t)(unsigned char system);
typedef void (*CG_DrawActive_t)(int local_client_num);
typedef XAssetEntry *(*DB_LinkXAsset_t)(XAssetEntry *new_entry, int allow_override);
typedef char *(*Scr_AddSourceBuffer_t)(const char *filename, const char *ext_filename, const char *code_pos,
                                       bool archive);

struct Addresses
{
    uint32_t com_init_dvars;
    uint32_t scr_shutdown_system;
    uint32_t cg_draw_active;
    uint32_t db_link_xasset;
    uint32_t scr_add_source_buffer;
};

extern const Addresses kAddresses;

} // namespace iw3_mp
} // namespace games
} // namespace codxe
