#pragma once

namespace qos
{
namespace mp
{

enum XAssetType : __int32
{
    ASSET_TYPE_CLIPMAP_SP = 12,
    ASSET_TYPE_CLIPMAP_MP = 13, // ASSET_TYPE_CLIPMAP_PVS
};

struct clipMap_t;

union XAssetHeader
{
    clipMap_t *clipMap;
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
    unsigned __int8 zoneIndex;
    bool inuse;
    unsigned __int16 nextHash;
    unsigned __int16 nextOverride;
    unsigned __int16 usageFrame;
};

// TODO: TBC...
enum DvarFlags : uint16_t
{
    DVAR_SERVERINFO =
        256, // this is sent (replicated) to all clients if you are host. Found in `GScr_MakeDvarServerInfo`
};

struct dvar_s
{
    const char *name;
    const char *description;
    unsigned __int16 flags;
    // unsigned __int8 type;
    // bool modified;
    // DvarValue current;
    // DvarValue latched;
    // DvarValue reset;
    // DvarLimits domain;
    // dvar_s *hashNext;
};
static_assert(offsetof(dvar_s, flags) == 8, "");

enum usercmd_button_bits
{
    CMD_BUTTON_ATTACK = 1,
    CMD_BUTTON_SPRINT = 2,
    CMD_BUTTON_USE_RELOAD = 32,
    CMD_BUTTON_CROUCH = 512,
    CMD_BUTTON_JUMP = 1024,
    CMD_BUTTON_ADS = 2048,
    CMD_BUTTON_FRAG = 16384,
    CMD_COVER = 524288, // Active when cover button is pressed but not yet in cover
    CMD_BUTTON_MELEE = 134217732,
    CMD_BUTTON_NEXTFIRETYPE = 268435456, // Toggles single/full auto
    CMD_BUTTON_MENU = 536870912,         // Menu is open
};

struct scr_entref_t
{
    uint16_t entnum;
    uint16_t classnum;
};

enum scr_builtin_type_t
{
    BUILTIN_ANY = 0x0,
    BUILTIN_DEVELOPER_ONLY = 0x1,
};

typedef void (*BuiltinFunction)();

struct BuiltinFunctionDef
{
    const char *actionString;
    BuiltinFunction actionFunc;
    scr_builtin_type_t type;
};

typedef void (*BuiltinMethod)(scr_entref_t entref);

struct BuiltinMethodDef
{
    const char *actionString;
    void (*actionFunc)(scr_entref_t);
    scr_builtin_type_t type;
};

struct playerState_s
{
    char pad_0[44];
    float velocity[3];
};
static_assert(offsetof(playerState_s, velocity) == 44, "");

struct gclient_s
{
    playerState_s ps;
    char pad_0[13196];
    int noclip;
    int ufo;
    char pad0[8];
    int buttons;
    char pad1[8];
    int buttonsSinceLastFrame;
    char pad2[256];
};
static_assert(sizeof(gclient_s) == 13540, "");
static_assert(offsetof(gclient_s, noclip) == 13252, "");
static_assert(offsetof(gclient_s, ufo) == 13256, "");
static_assert(offsetof(gclient_s, buttons) == 13268, "");
static_assert(offsetof(gclient_s, buttonsSinceLastFrame) == 13280, "");

struct gentity_s
{
    char pad_0[328];
    int contents;
    float absmin[3];
    float absmax[3];
    char pad_1[32];
    gclient_s *client;
    char pad_392[36];
    int flags;
    char pad_428[224];
};
static_assert(sizeof(gentity_s) == 656, "");
static_assert(offsetof(gentity_s, contents) == 328, "");
static_assert(offsetof(gentity_s, absmin) == 332, "");
static_assert(offsetof(gentity_s, absmax) == 344, "");
static_assert(offsetof(gentity_s, client) == 388, "");
static_assert(offsetof(gentity_s, flags) == 428, "");

struct cplane_s
{
    float normal[3];
    float dist;
    unsigned __int8 type;
    unsigned __int8 signbits;
    unsigned __int8 pad[2];
};

struct __declspec(align(2)) cbrushside_t
{
    cplane_s *plane;
    unsigned int materialNum;
    __int16 firstAdjacentSideOffset;
    unsigned __int8 edgeCount;
};

#pragma warning(disable : 4324)

struct __declspec(align(16)) cbrush_t
{
    float mins[3];
    int contents;
    float maxs[3];
    unsigned int numsides;
    cbrushside_t *sides;
    __int16 axialMaterialNum[2][3];
    unsigned __int8 *baseAdjacentSide;
    __int16 firstAdjacentSideOffsets[2][3];
    unsigned __int8 edgeCount[2][3];
};

#pragma warning(default : 4324)

static_assert(sizeof(cbrush_t) == 80, "");

struct clipMap_t
{
    const char *name;
    int isInUse;
    char pad[140];
    int numSubModels;
    int *cmodels;
    unsigned __int16 numBrushes;
    cbrush_t *brushes;
};
static_assert(offsetof(clipMap_t, name) == 0, "");
static_assert(offsetof(clipMap_t, isInUse) == 4, "");
static_assert(offsetof(clipMap_t, numSubModels) == 148, "");
static_assert(offsetof(clipMap_t, cmodels) == 152, "");
static_assert(offsetof(clipMap_t, numBrushes) == 156, "");
static_assert(offsetof(clipMap_t, brushes) == 160, "");

struct field_t
{
    int cursor;
    int scroll;
    int drawWidth;
    int widthInPixels;
    float charHeight;
    int fixedSize;
    char buffer[256];
};
static_assert(sizeof(field_t) == 0x118, "");

struct KeyState
{
    int down;
    int repeats;
    const char *binding;
};
static_assert(sizeof(KeyState) == 0xC, "");

enum LocSelInputState : __int32
{
    LOC_SEL_INPUT_NONE = 0x0,
    LOC_SEL_INPUT_CONFIRM = 0x1,
    LOC_SEL_INPUT_CANCEL = 0x2,
};

struct PlayerKeyState
{
    field_t chatField;
    int chat_team;
    int overstrikeMode;
    int anyKeyDown;
    KeyState keys[256];
    LocSelInputState locSelInputState;
};
static_assert(sizeof(PlayerKeyState) == 0xD28, "");

// TODO: TBC...
enum fieldtype_t : __int32
{
    F_INT = 0x0,
};

enum scr_classnum_t : __int32
{
    CLASS_NUM_ENTITY = 0x0,
    CLASS_NUM_HUDELEM = 0x1,
};

struct client_fields_s
{
    const char *name;
    int ofs;
    fieldtype_t type;
    void (*setter)(gclient_s *pSelf, const client_fields_s *pField);
    void (*getter)(gclient_s *pSelf, const client_fields_s *pField);
};
static_assert(sizeof(client_fields_s) == 20, "");

struct ent_field_t
{
    const char *name;
    int ofs;
    fieldtype_t type;
    void (*callback)(gentity_s *, int);
};
static_assert(sizeof(ent_field_t) == 16, "");

} // namespace mp
} // namespace qos
