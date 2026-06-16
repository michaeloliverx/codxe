#pragma once

#include "structs.h"

namespace t4
{
namespace mp
{
#define T4_MAX_CLIENTS 24

#define ANGLE2SHORT(x) ((int)((x) * 65536 / 360) & 65535)

enum
{
    T4_PITCH = 0,
    T4_YAW = 1,
    T4_ROLL = 2,
};

// Variables
static const int BUILTIN_FUNCTION_COUNT = 266;
static auto builtinFunctions = reinterpret_cast<BuiltinFunctionDef *>(0x82485C60);
static const int BUILTIN_PLAYER_METHOD_COUNT = 92;
static auto builtin_player_methods = reinterpret_cast<BuiltinFunctionDef *>(0x820275A0);
static auto client_fields = reinterpret_cast<client_fields_s *>(0x82026B80); // null-terminated array
static auto cm = reinterpret_cast<clipMap_t *>(0x82DD4F80);
static auto ent_fields = reinterpret_cast<ent_field_t *>(0x82035BF0); // null-terminated array
static auto g_entities = reinterpret_cast<gentity_s *>(0x82BAD1B0);
static auto level = reinterpret_cast<level_locals_t *>(0x82D6A440);
static auto svsHeader = reinterpret_cast<serverStaticHeader_t *>(0x84f85100);
static auto sv_maxclients = reinterpret_cast<dvar_s **>(0x83008C08);
static auto scrVarPub = reinterpret_cast<scrVarPub_t *>(0x85AC2F00);
static auto varclipMap_t = reinterpret_cast<clipMap_t **>(0x82756700);
static auto con = reinterpret_cast<Console *>(0x826B8C9C);
static auto g_consoleField = reinterpret_cast<field_t *>(0x826DCA40);
static auto clientConnectionStates = reinterpret_cast<connstate_t *>(0x826E8B10);
static const PlayerKeyState *playerKeys = reinterpret_cast<PlayerKeyState *>(0x826DCB60);
static ScreenPlacement &scrPlaceFull = *reinterpret_cast<ScreenPlacement *>(0x8274F6A8);

const int NUM_BSP_ONLY_SPAWNS = 14;
static auto s_bspOnlySpawns = reinterpret_cast<SpawnFuncEntry *>(0x82035CC0);

const int NUM_BSP_OR_DYNAMIC_SPAWNS = 6;
static auto s_bspOrDynamicSpawns = reinterpret_cast<SpawnFuncEntry *>(0x82035D30);

// Functions
static auto CG_BoldGameMessage = reinterpret_cast<void (*)(int localClientNum, const char *msg)>(0x8216EC88);
static auto CG_DrawActive = reinterpret_cast<void (*)(int localClientNum)>(0x82159560);
static auto CG_GameMessage = reinterpret_cast<void (*)(int localClientNum, const char *msg)>(0x8216EC68);
static auto CG_Init =
    reinterpret_cast<void (*)(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum)>(
        0x82171A30);

typedef int (*Con_OneTimeInit_t)();
static Con_OneTimeInit_t Con_OneTimeInit = reinterpret_cast<Con_OneTimeInit_t>(0x821A7360);

static auto CM_EntityString = reinterpret_cast<const char *(*)()>(0x82264788);
static auto CM_LoadMap = reinterpret_cast<void (*)(const char *name)>(0x821A4CB0);

static auto Cbuf_ExecuteBuffer =
    reinterpret_cast<void (*)(int localClientNum, int controllerIndex, const char *buffer)>(0x822711A8);
typedef void (*Cbuf_AddText_t)(int localClientNum, const char *text);
static Cbuf_AddText_t Cbuf_AddText = reinterpret_cast<Cbuf_AddText_t>(0x8226FF08);

typedef void (*Cmd_AddCommandInternal_t)(const char *cmdName, void (*function)(), cmd_function_s *allocedCmd);
static Cmd_AddCommandInternal_t Cmd_AddCommandInternal = reinterpret_cast<Cmd_AddCommandInternal_t>(0x822708D0);

typedef void (*Cmd_Init_t)();
static Cmd_Init_t Cmd_Init = reinterpret_cast<Cmd_Init_t>(0x82271570);

static auto CL_WritePacket = reinterpret_cast<void (*)(int localClientNum)>(0x821B0F30);

typedef void (*Com_Printf_t)(int channel, const char *fmt, ...);
static Com_Printf_t Com_Printf = reinterpret_cast<Com_Printf_t>(0x82271BE0);

static auto Com_InitDvars = reinterpret_cast<void (*)()>(0x82272BF8);

static auto DB_FindXAssetHeader =
    reinterpret_cast<XAssetHeader *(*)(const XAssetType type, const char *name, bool errorIfMissing, int waitTime)>(
        0x821EA5D8);
static auto DB_GetAllXAssetOfType_FastFile =
    reinterpret_cast<int (*)(XAssetType type, XAssetHeader *assets, int maxCount)>(0x821E7428);

static auto Dvar_FindMalleableVar = reinterpret_cast<dvar_s *(*)(const char *dvarName)>(0x822BDFE8);
static auto Dvar_GetBool = reinterpret_cast<dvar_s *(*)(const char *dvarName)>(0x822BE0E8);
static auto Dvar_GetString = reinterpret_cast<const char *(*)(const char *dvarName)>(0x822BE230);
static auto Dvar_RegisterBool =
    reinterpret_cast<dvar_s *(*)(const char *dvarName, bool value, DvarFlags flags, const char *description)>(
        0x822BF198);
static auto Dvar_RegisterString =
    reinterpret_cast<dvar_s *(*)(const char *dvarName, const char *value, DvarFlags flags, const char *description)>(
        0x822BF370);
static auto Dvar_SetBoolByName = reinterpret_cast<void (*)(const char *dvarName, bool value)>(0x822BFB20);

static auto GScr_AddFieldsForClient = reinterpret_cast<void (*)()>(0x8220A270);
static auto GScr_AllocString = reinterpret_cast<int (*)(const char *s)>(0x8222D368);

static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x822B9CC0);

static auto Load_clipMap_t = reinterpret_cast<void (*)(bool atStreamStart)>(0x821E4220);

static auto Menu_IsMenuOpenAndVisible =
    reinterpret_cast<bool (*)(int localClientNum, const char *menuName)>(0x8229F6C0);
static auto Menus_OpenByName = reinterpret_cast<void (*)(UiContext *dc, const char *menuName)>(0x822AFE88);

static auto Pmove = reinterpret_cast<void (*)(pmove_t *pm)>(0x8213FAE8);
static auto PmoveSingle = reinterpret_cast<void (*)(pmove_t *pm)>(0x8213ECE8);

static auto Player_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName)>(0x82211398);

static auto R_AddCmdDrawText =
    reinterpret_cast<void (*)(const char *text, int fontSize, Font_s *font, float x, float y, float scaleX,
                              float scaleY, float rotation, const float *color, int flags)>(0x82401C30);
static auto R_CheckDvarModified = reinterpret_cast<int (*)(const dvar_s *dvar)>(0x8240D860);

static auto ScriptEnt_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName)>(0x82244B50);
static auto Scr_AddArray = reinterpret_cast<void (*)(scriptInstance_t inst)>(0x82345C80);
static auto Scr_AddClassField =
    reinterpret_cast<void (*)(unsigned int classnum, const char *name, unsigned __int16 offset, scriptInstance_t inst)>(
        0x823414F0);
static auto Scr_AddInt = reinterpret_cast<void (*)(int value, scriptInstance_t inst)>(0x82345668);
typedef void (*Scr_AddString_t)(const char *value, scriptInstance_t inst);
static Scr_AddString_t Scr_AddString = reinterpret_cast<Scr_AddString_t>(0x823459A8);
typedef void (*Scr_AddUndefined_t)(scriptInstance_t inst);
static Scr_AddUndefined_t Scr_AddUndefined = reinterpret_cast<Scr_AddUndefined_t>(0x82345808);
static auto Scr_AddSourceBuffer =
    reinterpret_cast<char *(*)(scriptInstance_t inst, const char *filename, const char *extFilename,
                               const char *codePos, bool archive)>(0x82339EF8);
static auto Scr_GetEntity = reinterpret_cast<gentity_s *(*)(unsigned int index)>(0x82254018);
static auto Scr_Error = reinterpret_cast<void (*)(const char *error, scriptInstance_t inst)>(0x8234BE08);
static auto Scr_ErrorInternal = reinterpret_cast<void (*)()>(0x82343F78);
static auto Scr_GetFloat = reinterpret_cast<float (*)(unsigned int index, scriptInstance_t inst)>(0x8234B250);
static auto Scr_GetFunction = reinterpret_cast<BuiltinFunction (*)(const char **pName, int *type)>(0x822416B0);
static auto Scr_GetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs,
                                                            scriptInstance_t inst, unsigned int whichbits)>(0x82255100);
static auto Scr_GetInt =
    reinterpret_cast<int (*)(unsigned int index, scriptInstance_t inst, __int64 a3, __int64 a4)>(0x8234AFD0);
static auto Scr_GetNumParam = reinterpret_cast<unsigned int (*)(scriptInstance_t inst)>(0x82345650);
static auto Scr_GetObjectField =
    reinterpret_cast<void (*)(unsigned int classnum, int entnum, int offset, scriptInstance_t inst)>(0x822556B0);
static auto Scr_GetString = reinterpret_cast<const char *(*)(unsigned int index, scriptInstance_t inst)>(0x8234B550);
static auto Scr_GetVector =
    reinterpret_cast<void (*)(unsigned int index, float *vectorValue, scriptInstance_t inst, __int64 a4)>(0x8234B790);
static auto Scr_MakeArray = reinterpret_cast<void (*)(scriptInstance_t inst)>(0x82345BF8);
static auto Scr_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName, int *type)>(0x82241760);
static auto Scr_ObjectError = reinterpret_cast<void (*)(const char *error, scriptInstance_t inst)>(0x82345EF0);
static auto Scr_ParamError =
    reinterpret_cast<void (*)(unsigned int index, const char *error, scriptInstance_t inst)>(0x82345E70);
static auto Scr_SetClientField =
    reinterpret_cast<void (*)(gclient_s *client, int offset, scriptInstance_t inst)>(0x8220A2D0);
static auto Scr_SetGenericField =
    reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs, scriptInstance_t inst)>(0x82254E90);
typedef void *(*Scr_ShutdownSystem_t)(scriptInstance_t inst, int sys, int bComplete);
static Scr_ShutdownSystem_t Scr_ShutdownSystem = reinterpret_cast<Scr_ShutdownSystem_t>(0x82345388);

static auto String_Parse = reinterpret_cast<int (*)(const char **p, char *out, int len)>(0x822A8680);

typedef bool (*StringTable_GetAsset_t)(const char *filename, StringTable **tablePtr);
static StringTable_GetAsset_t StringTable_GetAsset = reinterpret_cast<StringTable_GetAsset_t>(0x822BB280);

static auto SV_ClientThink = reinterpret_cast<void (*)(client_t *cl, usercmd_s *cmd)>(0x82284D50);
typedef void (*SV_BotUserMove_t)(client_t *cl);
static SV_BotUserMove_t SV_BotUserMove = reinterpret_cast<SV_BotUserMove_t>(0x8228AB98);
static auto SV_GameSendServerCommand =
    reinterpret_cast<void (*)(int clientNum, svscmd_type type, const char *text)>(0x82285FA8);
static auto SV_LinkEntity = reinterpret_cast<void (*)(gentity_s *ent)>(0x82290C38);
static auto SV_SetBrushModel = reinterpret_cast<int (*)(gentity_s *ent)>(0x82286190);
static auto SV_UnlinkEntity = reinterpret_cast<void (*)(gentity_s *ent)>(0x82290BF0);
static auto Sys_SnapVector = reinterpret_cast<void (*)(float *v)>(0x822EA5A0);

bool SV_IsClientBot(unsigned int clientNum);

typedef void (*G_SelectWeaponIndex_t)(int clientNum, int iWeaponIndex);
static G_SelectWeaponIndex_t G_SelectWeaponIndex = reinterpret_cast<G_SelectWeaponIndex_t>(0x82260C38);

static auto UI_DrawBuildNumber = reinterpret_cast<void (*)()>(0x8229BCF8);
typedef int (*UI_Refresh_t)(int localClientNum);
static UI_Refresh_t UI_Refresh = reinterpret_cast<UI_Refresh_t>(0x822A4D48);
static auto UI_RunMenuScript =
    reinterpret_cast<void (*)(int localClientNum, const char **args, const char *actualScript)>(0x822A29B8);

static auto va = reinterpret_cast<char *(*)(char *format, ...)>(0x822C38D8);

typedef int (*Key_StringToKeynum_t)(const char *str);
static Key_StringToKeynum_t Key_StringToKeynum = reinterpret_cast<Key_StringToKeynum_t>(0x821B2938);

typedef void (*Field_AdjustScroll_t)(const ScreenPlacement *scrPlace, field_t *edit);
static Field_AdjustScroll_t Field_AdjustScroll = reinterpret_cast<Field_AdjustScroll_t>(0x821B2598);

typedef int (*Key_IsCatcherActive_t)(int localClientNum, int catcher);
static Key_IsCatcherActive_t Key_IsCatcherActive = reinterpret_cast<Key_IsCatcherActive_t>(0x821B4068);

typedef int (*Key_AddCatcher_t)(int localClientNum, int catcher);
static Key_AddCatcher_t Key_AddCatcher = reinterpret_cast<Key_AddCatcher_t>(0x821B40A8);

typedef int (*Key_RemoveCatcher_t)(int localClientNum, int catcher);
static Key_RemoveCatcher_t Key_RemoveCatcher = reinterpret_cast<Key_RemoveCatcher_t>(0x821B40D8);

void Con_ToggleConsole();
void Con_ToggleConsoleOutput();
void Con_Close(int localClientNum);
void Con_Bottom();
void Con_Top();
void Con_PageDown();
void Con_PageUp();
void Console_SubmitInput(int localClientNum);
void Console_HistoryNext();
void Console_HistoryPrev();
bool CL_IsConsoleKey(int key);

} // namespace mp
} // namespace t4
