#pragma once

#include <cstdint>

namespace codxe
{
namespace games
{
namespace iw3_mp
{

struct XModel;
struct Material;
struct GfxImage;
struct MapEnts;
struct Font_s;
struct RawFile;
struct StringTable;

enum XAssetType
{
    ASSET_TYPE_XMODELPIECES = 0x0,
    ASSET_TYPE_PHYSPRESET = 0x1,
    ASSET_TYPE_XANIMPARTS = 0x2,
    ASSET_TYPE_XMODEL = 0x3,
    ASSET_TYPE_MATERIAL = 0x4,
    ASSET_TYPE_PIXELSHADER = 0x5,
    ASSET_TYPE_TECHNIQUE_SET = 0x6,
    ASSET_TYPE_IMAGE = 0x7,
    ASSET_TYPE_SOUND = 0x8,
    ASSET_TYPE_SOUND_CURVE = 0x9,
    ASSET_TYPE_LOADED_SOUND = 0xA,
    ASSET_TYPE_CLIPMAP = 0xB,
    ASSET_TYPE_CLIPMAP_PVS = 0xC,
    ASSET_TYPE_COMWORLD = 0xD,
    ASSET_TYPE_GAMEWORLD_SP = 0xE,
    ASSET_TYPE_GAMEWORLD_MP = 0xF,
    ASSET_TYPE_MAP_ENTS = 0x10,
    ASSET_TYPE_GFXWORLD = 0x11,
    ASSET_TYPE_LIGHT_DEF = 0x12,
    ASSET_TYPE_UI_MAP = 0x13,
    ASSET_TYPE_FONT = 0x14,
    ASSET_TYPE_MENULIST = 0x15,
    ASSET_TYPE_MENU = 0x16,
    ASSET_TYPE_LOCALIZE_ENTRY = 0x17,
    ASSET_TYPE_WEAPON = 0x18,
    ASSET_TYPE_SNDDRIVER_GLOBALS = 0x19,
    ASSET_TYPE_FX = 0x1A,
    ASSET_TYPE_IMPACT_FX = 0x1B,
    ASSET_TYPE_AITYPE = 0x1C,
    ASSET_TYPE_MPTYPE = 0x1D,
    ASSET_TYPE_CHARACTER = 0x1E,
    ASSET_TYPE_XMODELALIAS = 0x1F,
    ASSET_TYPE_RAWFILE = 0x20,
    ASSET_TYPE_STRINGTABLE = 0x21,
    ASSET_TYPE_COUNT = 0x22,
    ASSET_TYPE_STRING = 0x22,
    ASSET_TYPE_ASSETLIST = 0x23,
};

union XAssetHeader
{
    XModel *model;
    Material *material;
    GfxImage *image;
    MapEnts *map_ents;
    Font_s *font;
    RawFile *rawfile;
    StringTable *string_table;
    void *data;
};

struct XAsset
{
    XAssetType type;
    XAssetHeader header;
};

struct XAssetEntry
{
    XAsset asset;
    uint8_t zone_index;
    bool in_use;
    uint16_t next_hash;
    uint16_t next_override;
    uint16_t usage_frame;
};

static_assert(sizeof(XAssetHeader) == 0x4, "");
static_assert(sizeof(XAsset) == 0x8, "");
static_assert(sizeof(XAssetEntry) == 0x10, "");

} // namespace iw3_mp
} // namespace games
} // namespace codxe
