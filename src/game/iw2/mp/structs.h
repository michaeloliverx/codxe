#pragma once

namespace iw2
{
namespace mp
{

// TODO: TBC...
enum pmflags_t : __int32
{
    PMF_PRONE = 1 << 0,
    PMF_DUCKED = 1 << 1,
    PMF_MANTLE = 1 << 2,
    PMF_NO_JUMP = 1 << 19,
};

struct scr_const_t
{
    unsigned __int16 _;
    unsigned __int16 allies;
    unsigned __int16 axis;
    unsigned __int16 current;
    unsigned __int16 damage;
    unsigned __int16 death;
    unsigned __int16 dlight;
    unsigned __int16 done;
    unsigned __int16 empty;
    unsigned __int16 entity;
    unsigned __int16 failed;
    unsigned __int16 fraction;
    unsigned __int16 goal;
    unsigned __int16 grenade;
    unsigned __int16 info_notnull;
    unsigned __int16 invisible;
    unsigned __int16 key1;
    unsigned __int16 key2;
    unsigned __int16 killanimscript;
    unsigned __int16 left;
    unsigned __int16 movedone;
    unsigned __int16 noclass;
    unsigned __int16 normal;
    unsigned __int16 pistol;
    unsigned __int16 plane_waypoint;
    unsigned __int16 player;
    unsigned __int16 position;
    unsigned __int16 primary;
    unsigned __int16 primaryb;
    unsigned __int16 prone;
    unsigned __int16 right;
    unsigned __int16 rocket;
    unsigned __int16 rotatedone;
    unsigned __int16 script_brushmodel;
    unsigned __int16 script_model;
    unsigned __int16 script_origin;
    unsigned __int16 spectator;
    unsigned __int16 stand;
    unsigned __int16 surfacetype;
    unsigned __int16 target_script_trigger;
    unsigned __int16 tempEntity;
    unsigned __int16 touch;
    unsigned __int16 trigger;
    unsigned __int16 trigger_use;
    unsigned __int16 trigger_use_touch;
    unsigned __int16 trigger_damage;
    unsigned __int16 trigger_lookat;
    unsigned __int16 truck_cam;
    unsigned __int16 worldspawn;
    unsigned __int16 binocular_enter;
    unsigned __int16 binocular_exit;
    unsigned __int16 binocular_fire;
    unsigned __int16 binocular_release;
    unsigned __int16 binocular_drop;
    unsigned __int16 begin;
    unsigned __int16 intermission;
    unsigned __int16 menuresponse;
    unsigned __int16 playing;
    unsigned __int16 none;
    unsigned __int16 dead;
    unsigned __int16 auto_change;
    unsigned __int16 manual_change;
    unsigned __int16 freelook;
    unsigned __int16 call_vote;
    unsigned __int16 vote;
    unsigned __int16 snd_enveffectsprio_level;
    unsigned __int16 snd_enveffectsprio_shellshock;
    unsigned __int16 snd_channelvolprio_holdbreath;
    unsigned __int16 snd_channelvolprio_pain;
    unsigned __int16 snd_channelvolprio_shellshock;
    unsigned __int16 tag_flash;
    unsigned __int16 tag_flash_11;
    unsigned __int16 tag_flash_2;
    unsigned __int16 tag_flash_22;
    unsigned __int16 tag_brass;
    unsigned __int16 j_head;
    unsigned __int16 tag_weapon;
    unsigned __int16 tag_player;
    unsigned __int16 tag_camera;
    unsigned __int16 tag_aim;
    unsigned __int16 tag_aim_animated;
    unsigned __int16 tag_origin;
    unsigned __int16 tag_butt;
    unsigned __int16 tag_weapon_right;
    unsigned __int16 back_low;
    unsigned __int16 back_mid;
    unsigned __int16 back_up;
    unsigned __int16 neck;
    unsigned __int16 head;
    unsigned __int16 pelvis;
    unsigned __int16 j_neck;
    unsigned __int16 j_helmet;
    unsigned __int16 j_head_2;
    unsigned __int16 j_spine4;
    unsigned __int16 j_head_3;
};

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
    char pad_0[0xC];
    int pm_flags;
    char pad_1[10152];
    int buttons; // 0x27B8
    char pad_27BC[0x08];
    int buttonsSinceLastFrame; // 0x27C4
};
static_assert(offsetof(gclient_s, pm_flags) == 0xC, "");
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
