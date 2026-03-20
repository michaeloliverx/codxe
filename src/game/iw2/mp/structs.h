#pragma once

namespace iw2
{
namespace mp
{

struct qkey_t
{
    int down;
    int repeats;
    const char *binding;
};
static_assert(sizeof(qkey_t) == 0x0C, "");

enum usercmd_button_bits
{
    CMD_BUTTON_ATTACK = 1,
    CMD_BUTTON_MELEE = 4,
    CMD_BUTTON_USE_RELOAD = 32,
    CMD_BUTTON_PRONE = 256,
    CMD_BUTTON_CROUCH = 512,
    CMD_BUTTON_JUMP = 1024,
    CMD_BUTTON_ADS = 4096,
    CMD_BUTTON_FRAG = 65536,
    CMD_BUTTON_SMOKE = 131072,
    CMD_BUTTON_MENU = 262144,      // Menu is open
    CMD_BUTTON_BINOCULARS = 53248, // Fully in binocular view
};

enum scr_builtin_type_t
{
    BUILTIN_ANY = 0x0,
    BUILTIN_DEVELOPER_ONLY = 0x1,
};

struct scr_entref_t
{
    uint16_t entnum;
    uint16_t classnum;
};

typedef void (*BuiltinFunction)();

struct BuiltinFunctionDef
{
    const char *actionString;
    BuiltinFunction actionFunc;
    scr_builtin_type_t type;
};
static_assert(sizeof(BuiltinFunctionDef) == 12, "");

typedef void (*BuiltinMethod)(scr_entref_t entref);

struct BuiltinMethodDef
{
    const char *actionString;
    BuiltinMethod actionFunc;
    scr_builtin_type_t type;
};
static_assert(sizeof(BuiltinMethodDef) == 12, "");

struct gclient_s
{
    char pad_0[0x27B8];
    int buttons; // 0x27B8
    char pad_27BC[0x08];
    int buttonsSinceLastFrame; // 0x27C4
};

static_assert(offsetof(gclient_s, buttons) == 0x27B8, "");
static_assert(offsetof(gclient_s, buttonsSinceLastFrame) == 0x27C4, "");

struct gentity_s
{
    char pad_0[0x158];
    gclient_s *client; // 0x0158
    char pad_15C[0x230 - 0x15C];
};
static_assert(sizeof(gentity_s) == 0x230, "");
static_assert(offsetof(gentity_s, client) == 0x158, "");

} // namespace mp
} // namespace iw2
