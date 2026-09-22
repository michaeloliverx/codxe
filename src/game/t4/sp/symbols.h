#pragma once

#include "structs.h"

namespace t4
{
namespace sp
{
// Data
static auto cm = reinterpret_cast<clipMap_t *>(0x831A0F00);
static auto g_clients = reinterpret_cast<gclient_s *>(0x82ABAC00);
static auto g_entities = reinterpret_cast<gentity_s *>(0x8293A280);
static auto level = reinterpret_cast<level_locals_t *>(0x82AC3930);

const int BSP_ONLY_SPAWNS_COUNT = 16;
static auto s_bspOnlySpawns = reinterpret_cast<SpawnFuncEntry *>(0x8202CA40);

const int BSP_OR_DYNAMIC_SPAWNS_COUNT = 11;
static auto s_bspOrDynamicSpawns = reinterpret_cast<SpawnFuncEntry *>(0x8202CAC0);

// Null-terminated array of client fields
static auto client_fields = reinterpret_cast<client_fields_s *>(0x82019DD0);

const int ENTITY_TYPE_COUNT = 19;
static auto entityTypeNames = reinterpret_cast<const char **>(0x824B60C8);

static auto varclipMap_t = reinterpret_cast<clipMap_t **>(0x8253F2F0);
static auto g_consoleField = reinterpret_cast<field_t *>(0x82538590);
static auto clientConnectionStates = reinterpret_cast<connstate_t *>(0x843C6880);
static auto conInitialized = reinterpret_cast<int *>(0x8251EBD0);
static auto conActiveLineCount = reinterpret_cast<int *>(0x8251EC00);
static auto conDisplayLineOffset = reinterpret_cast<int *>(0x8252EE0C);
static auto conOutputVisible = reinterpret_cast<unsigned __int8 *>(0x8252EE14);
static auto conVisibleLineCount = reinterpret_cast<int *>(0x8252EE1C);

// Functions
static auto BG_AddPredictableEventToPlayerstate =
    reinterpret_cast<void (*)(unsigned __int8 newEvent, unsigned __int8 eventParm, struct playerState_s *ps)>(
        0x820EFC08);

static auto CM_EntityString = reinterpret_cast<const char *(*)()>(0x82241C88);
static auto CL_CmdButtons = reinterpret_cast<void (*)(int a1, usercmd_s *a2)>(0x822E38F8);
static auto CL_WritePacket = reinterpret_cast<void (*)(int localClientNum)>(0x822E44B8);

static auto Cbuf_AddText = reinterpret_cast<void (*)(int localClientNum, const char *text)>(0x8224D8E0);

static auto Dvar_RegisterBool =
    reinterpret_cast<void *(*)(const char *dvarName, bool value, DvarFlags flags, const char *description)>(0x8228C588);
static auto Dvar_GetVariantString = reinterpret_cast<const char *(*)(const char *dvarName)>(0x8228B5E0);
static auto Dvar_SetBoolByName = reinterpret_cast<void (*)(const char *dvarName, bool value)>(0x8228CF10);
static auto Dvar_SetFromStringByName = reinterpret_cast<char *(*)(const char *dvarName, const char *value)>(0x8228D228);

static auto ClientScr_ReadOnly =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BC6F8);
static auto ClientScr_GetName = reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BC730);
static auto ClientScr_SetScore =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BC938);
static auto ClientScr_SetSessionState =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BC7A0);
static auto ClientScr_GetSessionState =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BC8B0);
static auto ClientScr_SetStatusIcon =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCA40);
static auto ClientScr_GetStatusIcon =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCA80);
static auto ClientScr_SetSpectatorClient =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BC9D8);
static auto ClientScr_SetArchiveTime =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCBB0);
static auto ClientScr_GetArchiveTime =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCC00);
static auto ClientScr_SetPSOffsetTime =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCC48);
static auto ClientScr_GetPSOffsetTime =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCC80);
static auto ClientScr_SetHeadIcon =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCAD0);
static auto ClientScr_GetHeadIcon =
    reinterpret_cast<void (*)(gclient_s *pSelf, const client_fields_s *pField)>(0x821BCB30);

static auto Com_Printf = reinterpret_cast<void (*)(int channel, const char *fmt, ...)>(0x8224F7A0);
static auto Com_PrintError = reinterpret_cast<void (*)(int channel, const char *fmt, ...)>(0x8224F8C0);
static auto Com_PrintMessage = reinterpret_cast<void (*)(int channel, const char *msg, int error)>(0x8224F804);
static auto CL_ConsolePrint = reinterpret_cast<void (*)(int localClientNum, int channel, const char *txt, int duration,
                                                        int pixelWidth, int flags)>(0x8214EA60);

static auto Con_OneTimeInit = reinterpret_cast<int (*)()>(0x8214DD00);

typedef void (*Field_AdjustScroll_t)(const ScreenPlacement *scrPlace, field_t *edit);
static Field_AdjustScroll_t Field_AdjustScroll = reinterpret_cast<Field_AdjustScroll_t>(0x82154920);

static auto Key_IsCatcherActive = reinterpret_cast<int (*)(int localClientNum, int catcher)>(0x821564E0);
static auto Key_AddCatcher = reinterpret_cast<int (*)(int localClientNum, int catcher)>(0x82156520);
static auto Key_RemoveCatcher = reinterpret_cast<int (*)(int localClientNum, int catcher)>(0x82156550);

typedef void *(*Hunk_AllocLowAlign_t)(size_t size, int alignment);
static Hunk_AllocLowAlign_t Hunk_AllocLowAlign = reinterpret_cast<Hunk_AllocLowAlign_t>(0x82286410);

static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x822862F8);

static auto Scr_AddInt = reinterpret_cast<void (*)(int value, scriptInstance_t inst)>(0x82342000);
static auto Scr_GetInt = reinterpret_cast<int (*)(unsigned int index, scriptInstance_t inst)>(0x823479F8);
static auto Scr_AddSourceBuffer =
    reinterpret_cast<char *(*)(scriptInstance_t inst, const char *filename, const char *extFilename,
                               const char *codePos, bool archive)>(0x82334558);
static auto Scr_Error = reinterpret_cast<void (*)(const char *error, scriptInstance_t inst)>(0x823489A8);
typedef void (*Scr_SetRuntimeError_t)(scriptInstance_t inst, const char *error, std::uint64_t source, int extraData);
static auto Scr_SetRuntimeError = reinterpret_cast<Scr_SetRuntimeError_t>(0x82342700);
static auto Scr_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName, int *type)>(0x821FE6F0);
static auto Scr_GetFunction = reinterpret_cast<BuiltinFunction (*)(const char **pName, int *type)>(0x821FCA38);
static auto Player_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName)>(0x821C39B8);
static auto Scr_GetNumParam = reinterpret_cast<unsigned int (*)(scriptInstance_t inst)>(0x82341FE8);
static auto Scr_ObjectError = reinterpret_cast<void (*)(const char *error, scriptInstance_t inst)>(0x823428F8);
static auto Scr_GetString = reinterpret_cast<const char *(*)(unsigned int index, scriptInstance_t inst)>(0x823480F0);
static auto Scr_GetVector =
    reinterpret_cast<void (*)(unsigned int index, float *vectorValue, scriptInstance_t inst, int errorOnUndefined)>(
        0x82348330);
static auto Scr_AddEntity = reinterpret_cast<void (*)(gentity_s *ent, scriptInstance_t inst)>(0x82211958);
static auto Scr_ParamError =
    reinterpret_cast<void (*)(unsigned int index, const char *error, scriptInstance_t inst)>(0x82342878);
static auto GetObjectType = reinterpret_cast<unsigned int (*)(unsigned int id, scriptInstance_t inst)>(0x82339DC8);
static auto Scr_GetEntity = reinterpret_cast<gentity_s *(*)(unsigned int index)>(0x82211A08);
static auto Scr_SetString =
    reinterpret_cast<void (*)(unsigned __int16 *to, unsigned int stringValue, scriptInstance_t inst)>(0x823389B0);

static auto G_Spawn = reinterpret_cast<gentity_s *(*)()>(0x82217D60);
static auto G_ModelIndex = reinterpret_cast<int (*)(const char *name)>(0x82216660);
static auto G_CallSpawnEntity = reinterpret_cast<bool (*)(gentity_s *ent)>(0x82211730);

static auto GScr_AddFieldsForClient = reinterpret_cast<void (*)()>(0x821BCC90);
static auto Scr_AddClassField =
    reinterpret_cast<void (*)(unsigned int classnum, const char *name, unsigned __int16 offset, scriptInstance_t inst)>(
        0x8233DE38);

static auto Scr_SetGenericField =
    reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs, scriptInstance_t inst)>(0x82212D38);
static auto Scr_GetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs,
                                                            scriptInstance_t inst, unsigned int whichbits)>(0x82212FA8);

static auto Scr_SetClientField =
    reinterpret_cast<void (*)(gclient_s *client, int offset, scriptInstance_t inst)>(0x821BCCF0);
static auto Scr_GetObjectField =
    reinterpret_cast<void (*)(unsigned int classnum, int entnum, int offset, scriptInstance_t inst)>(0x82213670);

static auto GScr_AllocString = reinterpret_cast<int (*)(const char *s)>(0x821E2CE0);

static auto SV_LinkEntity = reinterpret_cast<void (*)(gentity_s *gEnt)>(0x8225DB98);
static auto SV_UnlinkEntity = reinterpret_cast<void (*)(gentity_s *gEnt)>(0x8225DB50);
static auto SV_SetBrushModel = reinterpret_cast<void (*)(gentity_s *ent)>(0x8225CC20);
static auto SV_LocateGameData = reinterpret_cast<void (*)(gentity_s *gEnts, int numGEntities, int sizeofGEntity_t,
                                                          playerState_s *clients, int sizeofGameClient)>(0x8225CCA8);

static auto Load_clipMap_t = reinterpret_cast<void (*)(bool atStreamStart)>(0x82165290);

static auto UI_Refresh = reinterpret_cast<void (*)(int localClientNum)>(0x8226B7D0);
static auto Item_Slider_HandleKey = reinterpret_cast<int (*)(UiContext *dc, itemDef_s *item, int key)>(0x82271BB8);
static auto Menus_OpenByName = reinterpret_cast<int (*)(UiContext *dc, const char *menuName)>(0x822755B8);
static auto UI_PlayerStart = reinterpret_cast<void (*)()>(0x822675E8);

struct Font_s;

static auto scrPlaceFullUnsafe = reinterpret_cast<ScreenPlacement *>(0x8253F1D0);
static ScreenPlacement &scrPlaceFull = *scrPlaceFullUnsafe;

static auto UI_DrawText =
    reinterpret_cast<void (*)(const ScreenPlacement *scrPlace, const char *text, int maxChars, Font_s *font, float x,
                              float y, int horzAlign, int vertAlign, float scale, const float *color, int style)>(
        0x82266F28);

static auto DB_FindXAssetHeader =
    reinterpret_cast<XAssetHeader (*)(XAssetType type, const char *name, bool errorIfMissing, int waitTime)>(
        0x8216B688);
static auto DB_FindXAssetEntry = reinterpret_cast<XAssetEntry *(*)(XAssetType type, const char *name)>(0x82167FC0);

typedef void (*DB_LoadXAssets_t)(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
static auto DB_LoadXAssets = reinterpret_cast<DB_LoadXAssets_t>(0x8216B128);

typedef int (*Sys_CreateFile_t)(const char *filename, int desiredAccess, int shareMode, int securityAttributes,
                                int creationDisposition, int flagsAndAttributes);
static Sys_CreateFile_t Sys_CreateFile = reinterpret_cast<Sys_CreateFile_t>(0x823972F0);

typedef const char *(*DB_GetXAssetName_t)(const XAsset *asset);
static DB_GetXAssetName_t DB_GetXAssetName = reinterpret_cast<DB_GetXAssetName_t>(0x82159240);
typedef int (*DB_GetXAssetTypeSize_t)(XAssetType type);
static DB_GetXAssetTypeSize_t DB_GetXAssetTypeSize = reinterpret_cast<DB_GetXAssetTypeSize_t>(0x82159280);
static const char **g_assetNames = reinterpret_cast<const char **>(0x824B3AB8);
static int *g_poolSize = reinterpret_cast<int *>(0x824B3D40);
static void **DB_XAssetPool = reinterpret_cast<void **>(0x824B3F80);
static XZoneName *g_zoneNames = reinterpret_cast<XZoneName *>(0x8287BF50);
static unsigned int *g_zoneIndex = reinterpret_cast<unsigned int *>(0x825A3B24);
static bool *g_assetPoolsInitialized = reinterpret_cast<bool *>(0x8287C814);

// imageTrack is unused internally, it seems to be remnant from `useFastFile` dvar
// dev build could load raw font objects
static auto R_RegisterFont = reinterpret_cast<Font_s *(*)(const char *name, int imageTrack)>(0x8242B460);

static auto va = reinterpret_cast<char *(*)(char *format, ...)>(0x82294218);

} // namespace sp
} // namespace t4
