#pragma once

#include "structs.h"

namespace iw4
{
namespace mp_tu6
{
#define IW4_MAX_CLIENTS 18
#define IW4_ANGLE2SHORT(x) ((int)((x) * 65536 / 360) & 65535)

enum
{
    IW4_PITCH = 0,
    IW4_YAW = 1,
    IW4_ROLL = 2,
};

// Functions
static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x822FDF08);

static auto Cbuf_AddText = reinterpret_cast<void (*)(int localClientNum, const char *text)>(0x82275C60);

typedef void (*CL_CharEvent_t)(int localClientNum, int key);
static CL_CharEvent_t CL_CharEvent = reinterpret_cast<CL_CharEvent_t>(0x82182EC8);

typedef void (*CL_ConsoleCharEvent_t)(int localClientNum, int key);
static CL_ConsoleCharEvent_t CL_ConsoleCharEvent = reinterpret_cast<CL_ConsoleCharEvent_t>(0x82182E60);

typedef void (*CL_ConsolePrint_t)(int localClientNum, int channel, const char *txt, unsigned int duration,
                                  unsigned int pixelWidth, int flags);
static CL_ConsolePrint_t CL_ConsolePrint = reinterpret_cast<CL_ConsolePrint_t>(0x821754B8);

typedef void (*CL_KeyEvent_t)(int localClientNum, int key, int down, unsigned int time);
static CL_KeyEvent_t CL_KeyEvent = reinterpret_cast<CL_KeyEvent_t>(0x821826F8);

static auto CG_DrawActive = reinterpret_cast<void (*)(int localClientNum)>(0x82129270);
static auto CG_GameMessage = reinterpret_cast<void (*)(int localClientNum, const char *msg)>(0x8213DE38);

static auto Cmd_AddCommandInternal =
    reinterpret_cast<void (*)(const char *cmdName, void (*function)(), cmd_function_s *allocedCmd)>(0x82276758);

static auto Cmd_Init = reinterpret_cast<void (*)()>(0x82278048);

static auto Com_InitDvars = reinterpret_cast<void (*)()>(0x822804F8);

typedef void (*Com_Printf_t)(int channel, const char *fmt, ...);
static Com_Printf_t Com_Printf = reinterpret_cast<Com_Printf_t>(0x8227F448);

typedef void (*Com_PrintMessage_t)(int channel, const char *msg, int error);
static Com_PrintMessage_t Com_PrintMessage = reinterpret_cast<Com_PrintMessage_t>(0x8227F370);

typedef char (*Con_AllowAutoCompleteCycling_t)(char allowed);
static Con_AllowAutoCompleteCycling_t Con_AllowAutoCompleteCycling =
    reinterpret_cast<Con_AllowAutoCompleteCycling_t>(0x82177090);

typedef void (*Con_Bottom_t)();
static Con_Bottom_t Con_Bottom = reinterpret_cast<Con_Bottom_t>(0x82177AB8);

typedef int (*Con_CancelAutoComplete_t)();
static Con_CancelAutoComplete_t Con_CancelAutoComplete = reinterpret_cast<Con_CancelAutoComplete_t>(0x82177038);

typedef void (*Con_Close_t)(int localClientNum);
static Con_Close_t Con_Close = reinterpret_cast<Con_Close_t>(0x82177AD8);

typedef int (*Con_CycleAutoComplete_t)(int step);
static Con_CycleAutoComplete_t Con_CycleAutoComplete = reinterpret_cast<Con_CycleAutoComplete_t>(0x82176F48);

typedef void (*Con_Init_t)();
static Con_Init_t Con_Init = reinterpret_cast<Con_Init_t>(0x821735C0);

typedef int (*Con_IsActive_t)(int localClientNum);
static Con_IsActive_t Con_IsActive = reinterpret_cast<Con_IsActive_t>(0x82177C38);

typedef void (*Con_PageDown_t)();
static Con_PageDown_t Con_PageDown = reinterpret_cast<Con_PageDown_t>(0x82177A30);

typedef void (*Con_PageUp_t)();
static Con_PageUp_t Con_PageUp = reinterpret_cast<Con_PageUp_t>(0x821779C8);

typedef void (*Con_ToggleConsole_t)();
static Con_ToggleConsole_t Con_ToggleConsole = reinterpret_cast<Con_ToggleConsole_t>(0x82177D10);

typedef void (*Con_ToggleConsoleOutput_t)();
static Con_ToggleConsoleOutput_t Con_ToggleConsoleOutput = reinterpret_cast<Con_ToggleConsoleOutput_t>(0x82177528);

typedef void (*Con_Top_t)();
static Con_Top_t Con_Top = reinterpret_cast<Con_Top_t>(0x82177A78);

typedef void (*Console_Key_t)(int localClientNum, int key);
static Console_Key_t Console_Key = reinterpret_cast<Console_Key_t>(0x821821D0);

static auto DB_LinkXAssetEntry1 =
    reinterpret_cast<XAssetEntryPoolEntry *(*)(XAssetType type, XAssetHeader *header)>(0x821DE528);

typedef dvar_t *(*Dvar_RegisterBool_t)(const char *dvarName, bool value, unsigned __int16 flags,
                                       const char *description);
static Dvar_RegisterBool_t Dvar_RegisterBool = reinterpret_cast<Dvar_RegisterBool_t>(0x8230EE00);

typedef dvar_t *(*Dvar_RegisterFloat_t)(const char *dvarName, double value, double min, double max,
                                        unsigned __int16 flags, const char *description);
static Dvar_RegisterFloat_t Dvar_RegisterFloat = reinterpret_cast<Dvar_RegisterFloat_t>(0x8230EE90);

typedef dvar_t *(*Dvar_RegisterString_t)(const char *dvarName, const char *value, unsigned __int16 flags,
                                         const char *description);
static Dvar_RegisterString_t Dvar_RegisterString = reinterpret_cast<Dvar_RegisterString_t>(0x8230F010);

static auto Dvar_SetString = reinterpret_cast<void (*)(const dvar_t *dvar, const char *value)>(0x8230F7D8);

typedef void (*Field_AdjustScroll_t)(const ScreenPlacement *scrPlace, field_t *edit);
static Field_AdjustScroll_t Field_AdjustScroll = reinterpret_cast<Field_AdjustScroll_t>(0x8217FA98);

typedef int (*Field_CharEvent_t)(int localClientNum, const ScreenPlacement *scrPlace, field_t *edit, int ch);
static Field_CharEvent_t Field_CharEvent = reinterpret_cast<Field_CharEvent_t>(0x82182028);

typedef int (*Field_KeyDownEvent_t)(int localClientNum, const ScreenPlacement *scrPlace, field_t *edit, int key);
static Field_KeyDownEvent_t Field_KeyDownEvent = reinterpret_cast<Field_KeyDownEvent_t>(0x82181DB8);

typedef int (*Key_AddCatcher_t)(int localClientNum, int orMask);
static Key_AddCatcher_t Key_AddCatcher = reinterpret_cast<Key_AddCatcher_t>(0x82181AC8);

typedef int (*Key_IsCatcherActive_t)(int localClientNum, int mask);
static Key_IsCatcherActive_t Key_IsCatcherActive = reinterpret_cast<Key_IsCatcherActive_t>(0x82181AA0);

typedef int (*Key_RemoveCatcher_t)(int localClientNum, int andMask);
static Key_RemoveCatcher_t Key_RemoveCatcher = reinterpret_cast<Key_RemoveCatcher_t>(0x82181AE8);

typedef int (*Key_SetCatcher_t)(int localClientNum, int catcher);
static Key_SetCatcher_t Key_SetCatcher = reinterpret_cast<Key_SetCatcher_t>(0x82181B20);

typedef Material *(*Material_RegisterHandle_t)(const char *name);
static Material_RegisterHandle_t Material_RegisterHandle = reinterpret_cast<Material_RegisterHandle_t>(0x823C2FF8);

static auto R_CheckDvarModified = reinterpret_cast<int (*)(const dvar_t *dvar)>(0x823DDD78);

typedef void (*R_AddCmdDrawStretchPic_t)(float x, float y, float w, float h, float s0, float t0, float s1, float t1,
                                         const float *color, Material *material);
static R_AddCmdDrawStretchPic_t R_AddCmdDrawStretchPic = reinterpret_cast<R_AddCmdDrawStretchPic_t>(0x823C6DB0);

typedef void (*R_AddCmdDrawText_t)(const char *text, int maxChars, iw4::mp_tu6::Font_s *font, float x, float y,
                                   float xScale, float yScale, float rotation, const float *color, int style);
static R_AddCmdDrawText_t R_AddCmdDrawText = reinterpret_cast<R_AddCmdDrawText_t>(0x823C7690);

static auto R_RegisterFont = reinterpret_cast<Font_s *(*)(const char *name)>(0x823C2798);

typedef int (*R_TextWidth_t)(const char *text, int maxChars, Font_s *font);
static R_TextWidth_t R_TextWidth = reinterpret_cast<R_TextWidth_t>(0x823C28F8);

static auto Scr_AddSourceBuffer =
    reinterpret_cast<char *(*)(const char *filename, const char *extFilename)>(0x8229F2C8);

static auto Scr_AddClassField =
    reinterpret_cast<void (*)(ClassNum classnum, const char *name, unsigned __int16 offset)>(0x822A9B98);
static auto GScr_AddFieldsForClient = reinterpret_cast<void (*)()>(0x8221B238);
static auto Scr_SetClientField = reinterpret_cast<void (*)(gclient_s *client, int offset)>(0x8221B290);
static auto Scr_SetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs)>(0x8225A698);
static auto Scr_GetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs)>(0x8225A7B8);
static auto Scr_GetObjectField = reinterpret_cast<void (*)(ClassNum classnum, int entnum, int offset)>(0x8225ABF0);

static auto Scr_GetString = reinterpret_cast<const char *(*)(unsigned int index)>(0x822B33A8);
static auto Scr_AddInt = reinterpret_cast<void (*)(int value)>(0x822ADD18);
static auto Scr_GetInt = reinterpret_cast<int (*)(unsigned int index)>(0x822B2D70);
typedef double (*Scr_GetFloat_t)(unsigned int index);
static Scr_GetFloat_t Scr_GetFloat = reinterpret_cast<Scr_GetFloat_t>(0x822B30D0);
typedef void (*Scr_AddUndefined_t)();
static Scr_AddUndefined_t Scr_AddUndefined = reinterpret_cast<Scr_AddUndefined_t>(0x822ADE98);
typedef void (*Scr_AddString_t)(const char *value);
static Scr_AddString_t Scr_AddString = reinterpret_cast<Scr_AddString_t>(0x822ADFF0);
typedef void (*Scr_AddEntity_t)(gentity_s *ent);
static Scr_AddEntity_t Scr_AddEntity = reinterpret_cast<Scr_AddEntity_t>(0x82259A60);
static auto Scr_GetVector = reinterpret_cast<void (*)(unsigned int index, float *vectorValue)>(0x822B35B8);
static auto Scr_Error = reinterpret_cast<void (*)(const char *error)>(0x822AE470);
static auto GetEntity = reinterpret_cast<gentity_s *(*)(scr_entref_t entref)>(0x8223F4D0);
typedef gentity_s *(*GetPlayerEntity_t)(scr_entref_t entref);
static GetPlayerEntity_t GetPlayerEntity = reinterpret_cast<GetPlayerEntity_t>(0x8223F540);
typedef void (*Scr_ParamError_t)(unsigned int index, const char *error);
static Scr_ParamError_t Scr_ParamError = reinterpret_cast<Scr_ParamError_t>(0x822AE600);
static auto Scr_ObjectError = reinterpret_cast<void (*)(const char *error)>(0x822AE668);

static auto Scr_RegisterFunction = reinterpret_cast<void (*)(int func, const char *name)>(0x822963C0);

static auto Scr_GetFunction =
    reinterpret_cast<BuiltinFunction (*)(const char **pName, scr_builtin_type_t *type)>(0x82254C38);
static auto Scr_GetMethod =
    reinterpret_cast<BuiltinMethod (*)(const char **pName, scr_builtin_type_t *type)>(0x82254D88);

static auto Scr_GetNumParam = reinterpret_cast<unsigned int (*)()>(0x822ADC88);

typedef void (*PlayerCmd_GetGuid_t)(scr_entref_t entref);
static PlayerCmd_GetGuid_t PlayerCmd_GetGuid = reinterpret_cast<PlayerCmd_GetGuid_t>(0x82223C18);

typedef int *(*G_SelectWeaponIndex_t)(int clientNum, int iWeaponIndex);
static G_SelectWeaponIndex_t G_SelectWeaponIndex = reinterpret_cast<G_SelectWeaponIndex_t>(0x82261410);

typedef void (*SV_ClientThink_t)(client_t *cl, usercmd_s *cmd);
static SV_ClientThink_t SV_ClientThink = reinterpret_cast<SV_ClientThink_t>(0x822BC778);

typedef void (*SV_BotUserMove_t)(client_t *cl);
static SV_BotUserMove_t SV_BotUserMove = reinterpret_cast<SV_BotUserMove_t>(0x822C3208);

typedef BOOL (*SV_IsClientBot_t)(int clientNum);
static SV_IsClientBot_t SV_IsClientBot = reinterpret_cast<SV_IsClientBot_t>(0x822C3B10);

typedef gentity_s *(*SV_AddTestClient_t)();
static SV_AddTestClient_t SV_AddTestClient = reinterpret_cast<SV_AddTestClient_t>(0x822BDCB8);

typedef void (*Scr_ShutdownSystem_t)(unsigned __int8 sys);
static Scr_ShutdownSystem_t Scr_ShutdownSystem = reinterpret_cast<Scr_ShutdownSystem_t>(0x822A36E8);

typedef void (*BG_ComputeAndApplyWeaponMovement_IdleAngles_t)(weaponState_t *ws, float *angles);
static BG_ComputeAndApplyWeaponMovement_IdleAngles_t BG_ComputeAndApplyWeaponMovement_IdleAngles =
    reinterpret_cast<BG_ComputeAndApplyWeaponMovement_IdleAngles_t>(0x82116638);

typedef void (*BG_CalculateViewMovement_Angles_Idle_t)(viewState_t *vs, float *angles);
static BG_CalculateViewMovement_Angles_Idle_t BG_CalculateViewMovement_Angles_Idle =
    reinterpret_cast<BG_CalculateViewMovement_Angles_Idle_t>(0x82118198);

static auto Weapon_RocketLauncher_Fire =
    reinterpret_cast<gentity_s *(*)(gentity_s * ent, unsigned int weaponIndex, double spread, weaponParms *wp,
                                    weaponParms *gunVel, struct lockonFireParms *lockParms,
                                    lockonFireParms *magicBullet)>(0x82260C90);

typedef const ScreenPlacement *(*ScrPlace_GetFullPlacement_t)();
static ScrPlace_GetFullPlacement_t ScrPlace_GetFullPlacement =
    reinterpret_cast<ScrPlace_GetFullPlacement_t>(0x821AB790);

static auto ScrPlace_GetUnsafeFullPlacement = reinterpret_cast<const ScreenPlacement *(*)()>(0x821AB7A0);

static auto SCR_DrawScreenField = reinterpret_cast<void (*)(int localClientNum, int refreshedUI)>(0x8218E800);

typedef unsigned int (*Sys_Milliseconds_t)();
static Sys_Milliseconds_t Sys_Milliseconds = reinterpret_cast<Sys_Milliseconds_t>(0x823401C8);

static auto UI_DrawBuildNumber = reinterpret_cast<void (*)(int localClientNum)>(0x822DAC70);
static auto UI_DrawText =
    reinterpret_cast<void (*)(const ScreenPlacement *scrPlace, const char *text, int maxChars, Font_s *font, double x,
                              double y, int horzAlign, int vertAlign, double scale, const float *color, int style)>(
        0x822DA678);
// static auto UI_RefreshViewport = reinterpret_cast<void (*)(int localClientNum)>(0x822E4100);

static auto va = reinterpret_cast<char *(*)(const char *format, ...)>(0x823160A8);

// Data
static auto cgArray = reinterpret_cast<cg_s **>(0x824C5B64);
static auto clients = reinterpret_cast<clientActive_t **>(0x825A8B6C);
static auto clientUIActives = reinterpret_cast<clientUIActive_t *>(0x825A5918);
static auto con = reinterpret_cast<Console *>(0x824DF4A8);
static auto g_consoleField = reinterpret_cast<field_t *>(0x82502FF8);
static auto historyEditLines = reinterpret_cast<field_t *>(0x825065B8);
static auto playerKeys = reinterpret_cast<PlayerKeyState *>(0x82503118);

static auto CG_GetPredictedPlayerState = reinterpret_cast<playerState_s *(*)(int localClientNum)>(0x8213DE18);
static auto CL_CreateNewCommands = reinterpret_cast<void (*)(int localClientNum)>(0x8217E540);

static auto cm = reinterpret_cast<clipMap_t *>(0x83052680);
static auto fields = reinterpret_cast<client_fields_s *>(0x82027518);
static auto g_entities = reinterpret_cast<gentity_s *>(0x82E2A580);
static auto level = reinterpret_cast<level_locals_t *>(0x82FF2F08);
static auto sharedUiInfo = reinterpret_cast<sharedUiInfo_t *>(0x836A3AC0);
static auto svsHeader = reinterpret_cast<serverStaticHeader_t *>(0x8367A010);

} // namespace mp_tu6
} // namespace iw4
