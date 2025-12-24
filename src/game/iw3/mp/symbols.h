#pragma once

#include "structs.h"

namespace iw3
{
namespace mp
{
// Functions
static auto AngleDelta = reinterpret_cast<float (*)(float a1, float a2)>(0x821DABC0);
static auto AngleNormalize180 = reinterpret_cast<float (*)(float angle)>(0x820A0088);
static auto AngleNormalize360 = reinterpret_cast<float (*)(float angle)>(0x821D9DA8);

static auto Cbuf_AddText = reinterpret_cast<void (*)(int localClientNum, const char *text)>(0x82239FD0);
static auto Cbuf_ExecuteBuffer =
    reinterpret_cast<void (*)(int localClientNum, int controllerIndex, const char *buffer)>(0x8223AAE8);

typedef void (*CG_DrawActive_t)(int localClientNum);
static CG_DrawActive_t CG_DrawActive = reinterpret_cast<CG_DrawActive_t>(0x8231E6E0);

static auto CG_GameMessage = reinterpret_cast<void (*)(int localClientNum, const char *msg)>(0x8230AAF0);
static auto CG_GetPredictedPlayerState = reinterpret_cast<const playerState_s *(*)(int localClientNum)>(0x82309120);
static auto CG_RegisterGraphics = reinterpret_cast<void (*)(int localClientNum, const char *mapname)>(0x8230D858);

static auto CL_CM_LoadMap = reinterpret_cast<void (*)(const char *name)>(0x822E6E60);
static auto CM_LoadMap = reinterpret_cast<void (*)(const char *name, unsigned int *checksum)>(0x82243940);
static auto CL_ConsolePrint = reinterpret_cast<void (*)(int localClientNum, int channel, const char *txt, int duration,
                                                        int pixelWidth, int flags)>(0x822E4D18);
static auto CL_GamepadButtonEvent =
    reinterpret_cast<void (*)(int localClientNum, int controllerIndex, int key, int down, unsigned int time)>(
        0x822DD1E8);

static auto ClientCommand = reinterpret_cast<void (*)(int clientNum)>(0x8227DCF0);
static auto ClientThink = reinterpret_cast<void (*)(int clientNum)>(0x822886E8);

static auto ClientScr_ReadOnly = reinterpret_cast<ClientFieldFunc>(0x82366748);
static auto ClientScr_GetName = reinterpret_cast<ClientFieldFunc>(0x82366C28);
static auto ClientScr_SetSessionTeam = reinterpret_cast<ClientFieldFunc>(0x82366EC0);
static auto ClientScr_GetSessionTeam = reinterpret_cast<ClientFieldFunc>(0x82366BB0);
static auto ClientScr_SetSessionState = reinterpret_cast<ClientFieldFunc>(0x82366938);
static auto ClientScr_GetSessionState = reinterpret_cast<ClientFieldFunc>(0x82366B38);
static auto ClientScr_SetMaxHealth = reinterpret_cast<ClientFieldFunc>(0x823666A8);
static auto ClientScr_SetScore = reinterpret_cast<ClientFieldFunc>(0x82366670);
static auto ClientScr_SetStatusIcon = reinterpret_cast<ClientFieldFunc>(0x82366E58);
static auto ClientScr_GetStatusIcon = reinterpret_cast<ClientFieldFunc>(0x82366DF8);
static auto ClientScr_SetHeadIcon = reinterpret_cast<ClientFieldFunc>(0x82366D60);
static auto ClientScr_GetHeadIcon = reinterpret_cast<ClientFieldFunc>(0x82366CC8);
static auto ClientScr_SetHeadIconTeam = reinterpret_cast<ClientFieldFunc>(0x823667E8);
static auto ClientScr_GetHeadIconTeam = reinterpret_cast<ClientFieldFunc>(0x82366AB0);
static auto ClientScr_SetSpectatorClient = reinterpret_cast<ClientFieldFunc>(0x82366610);
static auto ClientScr_SetKillCamEntity = reinterpret_cast<ClientFieldFunc>(0x823665B0);
static auto ClientScr_SetArchiveTime = reinterpret_cast<ClientFieldFunc>(0x82366568);
static auto ClientScr_GetArchiveTime = reinterpret_cast<ClientFieldFunc>(0x82366A68);
static auto ClientScr_SetPSOffsetTime = reinterpret_cast<ClientFieldFunc>(0x82366530);
static auto ClientScr_GetPSOffsetTime = reinterpret_cast<ClientFieldFunc>(0x82366A60);

static auto Cmd_AddCommandInternal =
    reinterpret_cast<void (*)(const char *cmdName, void (*function)(), cmd_function_s *allocedCmd)>(0x8223ADE0);
static auto Cmd_ExecFromFastFile =
    reinterpret_cast<bool (*)(int localClientNum, int controllerIndex, const char *filename)>(0x8223AF40);
static auto Cmd_ExecuteSingleCommand =
    reinterpret_cast<void (*)(int localClientNum, int controllerIndex, const char *text)>(0x8223A7A0);

static auto CheatsOk = reinterpret_cast<int (*)(gentity_s *ent)>(0x8227BF40);

static auto Com_Printf = reinterpret_cast<void (*)(conChannel_t channel, const char *fmt, ...)>(0x82237000);
static auto Com_PrintError = reinterpret_cast<void (*)(conChannel_t channel, const char *fmt, ...)>(0x82235C50);
static auto Com_PrintWarning = reinterpret_cast<void (*)(conChannel_t channel, const char *fmt, ...)>(0x822356B8);

static auto DB_EnumXAssets_FastFile = reinterpret_cast<void (*)(
    XAssetType type, void (*func)(void *asset, void *inData), void *inData, bool includeOverride)>(0x8229ED48);
static auto DB_FindXAssetEntry =
    reinterpret_cast<XAssetEntryPoolEntry *(*)(XAssetType type, const char *name)>(0x8229EB98);
static auto DB_FindXAssetHeader =
    reinterpret_cast<XAssetHeader *(*)(const XAssetType type, const char *name)>(0x822A0298);
static auto DB_GetAllXAssetOfType_FastFile =
    reinterpret_cast<int (*)(XAssetType type, XAssetHeader *assets, int maxCount)>(0x8229E8E0);

static auto Dvar_FindMalleableVar = reinterpret_cast<dvar_s *(*)(const char *dvarName)>(0x821D4C10);
static auto Dvar_GetBool = reinterpret_cast<bool (*)(const char *dvarName)>(0x821D15D8);
static auto Dvar_GetInt = reinterpret_cast<int (*)(const char *dvarName)>(0x821D1570);
static auto Dvar_GetString = reinterpret_cast<const char *(*)(const char *dvarName)>(0x821D1478);
static auto Dvar_RegisterBool =
    reinterpret_cast<dvar_s *(*)(const char *dvarName, bool value, unsigned __int16 flags, const char *description)>(
        0x821D5180);
static auto Dvar_RegisterColor =
    reinterpret_cast<dvar_s *(*)(const char *dvarName, double r, double g, double b, double a, unsigned __int16 flags,
                                 const char *description)>(0x821D4D98);
static auto Dvar_RegisterEnum =
    reinterpret_cast<dvar_s *(*)(const char *dvarName, const char **valueList, unsigned __int16 defaultIndex,
                                 unsigned __int16 flags, const char *description)>(0x821D4F88);
static auto Dvar_RegisterInt =
    reinterpret_cast<dvar_s *(*)(const char *dvarName, int value, int min, int max, unsigned __int16 flags,
                                 const char *description)>(0x821D5138);
static auto Dvar_RegisterString =
    reinterpret_cast<dvar_s *(*)(const char *dvarName, const char *value, DvarFlags flags, const char *description)>(
        0x821D1040);

typedef gentity_s *(*GetEntity_t)(scr_entref_t entref);
static GetEntity_t GetEntity = reinterpret_cast<GetEntity_t>(0x82257F30);

typedef gentity_s *(*GetPlayerEntity_t)(scr_entref_t entref);
static GetPlayerEntity_t GetPlayerEntity = reinterpret_cast<GetPlayerEntity_t>(0x82259258);

static auto G_SetAngle = reinterpret_cast<void (*)(gentity_s *ent, float *origin)>(0x8224AA98);
static auto G_SetLastServerTime = reinterpret_cast<void (*)(int clientNum, int lastServerTime)>(0x82285D08);
static auto G_SetOrigin = reinterpret_cast<void (*)(gentity_s *ent, float *origin)>(0x8224AAF0);

static auto I_strnicmp = reinterpret_cast<int (*)(const char *s0, const char *s1, int n)>(0x821CDA98);

static auto Load_MapEntsPtr = reinterpret_cast<void (*)()>(0x822A9648);

static auto Menus_OpenByName = reinterpret_cast<void (*)(UiContext *dc, const char *menuName)>(0x821E5B38);

static auto PM_FoliageSounds = reinterpret_cast<void (*)(pmove_t *pm)>(0x82335E90);
static auto Pmove = reinterpret_cast<void (*)(pmove_t *pm)>(0x8233B470);
static auto PmoveSingle = reinterpret_cast<void (*)(pmove_t *pm)>(0x8233A938);

static auto R_AddCmdDrawText =
    reinterpret_cast<void (*)(const char *text, int maxChars, Font_s *font, double x, double y, double xScale,
                              double yScale, double rotation, const float *color, int style)>(0x8216C0B8);
static auto R_CheckDvarModified = reinterpret_cast<int (*)(const dvar_s *dvar)>(0x8234AF50);
static auto R_DrawAllDynEnt = reinterpret_cast<void (*)(const GfxViewInfo *viewInfo)>(0x8215FF98);
static auto R_GetImageList = reinterpret_cast<void (*)(ImageList *imageList)>(0x82152A58);
static auto R_RegisterFont = reinterpret_cast<Font_s *(*)(const char *name)>(0x8216EC00);
static auto R_StreamLoadFileSynchronously =
    reinterpret_cast<int (*)(const char *filename, unsigned int bytesToRead, unsigned __int8 *outData)>(0x82151510);

static auto SetClientViewAngle = reinterpret_cast<void (*)(gentity_s *ent, float *angle)>(0x82284C60);

static auto SCR_DrawSmallStringExt =
    reinterpret_cast<void (*)(unsigned int x, unsigned int y, const char *string, const float *setColor)>(0x822C9B88);

static auto Scr_AddArray = reinterpret_cast<void (*)()>(0x82210538);
static auto Scr_AddInt = reinterpret_cast<void (*)(int value)>(0x822111C0);
static auto Scr_AddString = reinterpret_cast<void (*)(const char *value)>(0x82210F28);
static auto Scr_Error = reinterpret_cast<void (*)(const char *error)>(0x8220F6F0);
static auto Scr_GetEntity = reinterpret_cast<gentity_s *(*)(scr_entref_t * entref)>(0x8224EE68);
static auto Scr_GetFunction = reinterpret_cast<BuiltinFunction (*)(const char **pName, int *type)>(0x82256ED0);
static auto Scr_GetInt = reinterpret_cast<int (*)(unsigned int index)>(0x8220FD10);
static auto Scr_GetString = reinterpret_cast<char *(*)(unsigned int index)>(0x82211390);
static auto Scr_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName, int *type)>(0x822570E0);
static auto Scr_GetVector = reinterpret_cast<void (*)(unsigned int index, float *vectorValue)>(0x8220FA88);
static auto Scr_GetNumParam = reinterpret_cast<unsigned int (*)()>(0x8220F3E0);
static auto Scr_MakeArray = reinterpret_cast<void (*)()>(0x82210CA0);
static auto Scr_ObjectError = reinterpret_cast<void (*)(const char *error)>(0x8220FDD0);
static auto Scr_ParamError = reinterpret_cast<void (*)(unsigned int index, const char *error)>(0x8220FE08);

typedef void (*SV_ClientThink_t)(client_t *cl, usercmd_s *cmd);
static SV_ClientThink_t SV_ClientThink = reinterpret_cast<SV_ClientThink_t>(0x82208448);

typedef void (*SV_BotUserMove_t)(client_t *cl);
static SV_BotUserMove_t SV_BotUserMove = reinterpret_cast<SV_BotUserMove_t>(0x822009A8);

typedef playerState_s *(*SV_GameClientNum_t)(int num);
static SV_GameClientNum_t SV_GameClientNum = reinterpret_cast<SV_GameClientNum_t>(0x82204250);

static auto SV_Cmd_ArgvBuffer = reinterpret_cast<void (*)(int arg, char *buffer, int bufferLength)>(0x82239F48);
static auto SV_GameSendServerCommand =
    reinterpret_cast<void (*)(int clientNum, svscmd_type type, const char *text)>(0x82204BB8);
static auto SV_LinkEntity = reinterpret_cast<void (*)(gentity_s *ent)>(0x82355A00);
static auto SV_SendServerCommand =
    reinterpret_cast<void (*)(client_t *cl, svscmd_type type, const char *fmt, ...)>(0x821FFE30);
static auto SV_SetBrushModel = reinterpret_cast<int (*)(gentity_s *ent)>(0x82205050);
static auto SV_UnlinkEntity = reinterpret_cast<void (*)(gentity_s *ent)>(0x82355F08);

static auto Sys_SnapVector = reinterpret_cast<void (*)(float *result)>(0x821A3BD0);

static auto TeleportPlayer = reinterpret_cast<void (*)(gentity_s *player, float *origin, float *angles)>(0x8226F408);

static auto UI_DrawBuildNumber = reinterpret_cast<void (*)(int localClientNum)>(0x821EBB30);
static auto UI_DrawTextExt =
    reinterpret_cast<void (*)(const ScreenPlacement *scrPlace, const char *text, int maxChars, Font_s *font, double x,
                              double y, int horzAlign, int vertAlign, double scale, const float *color, int style)>(
        0x821EB858);
static auto UI_OpenMenu = reinterpret_cast<void (*)(int localClientNum, const char *menuName)>(0x821EA1E0);
static auto UI_Refresh = reinterpret_cast<void (*)(int localClientNum)>(0x821F2F28);
static auto UI_SafeTranslateString = reinterpret_cast<const char *(*)(char *reference)>(0x821ECA78);

static auto va = reinterpret_cast<char *(*)(char *format, ...)>(0x821CD858);

static auto Jump_Check = reinterpret_cast<int (*)(pmove_t *pm, pml_t *pml)>(0x82341480);
static auto BG_CalculateWeaponPosition_IdleAngles =
    reinterpret_cast<void (*)(weaponState_t *ws, float *angles)>(0x8232CA78);
static auto BG_CalculateView_IdleAngles = reinterpret_cast<void (*)(viewState_t *vs, float *angles)>(0x8232C840);

static auto GScr_AddFieldsForClient = reinterpret_cast<void (*)()>(0x82366780);
static auto Scr_AddClassField =
    reinterpret_cast<void (*)(unsigned int classnum, const char *name, unsigned __int16 offset)>(0x8221B2A0);

static auto Scr_SetEntityField = reinterpret_cast<void (*)(int entnum, int offset, int value)>(0x8224FA28);
static auto Scr_GetEntityField = reinterpret_cast<void (*)(int entnum, int offset, int *value)>(0x8224F5C8);

static auto Scr_SetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs)>(0x8224F6B0);
static auto Scr_GetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs)>(0x8224F1A0);

static auto BG_FindWeaponIndexForName = reinterpret_cast<unsigned int (*)(const char *name)>(0x8232DC38);

static auto CL_CreateNewCommands = reinterpret_cast<void (*)(int localClientNum)>(0x822DCAE8);
static auto CL_FinishMove = reinterpret_cast<void (*)(int localClientNum, usercmd_s *cmd)>(0x822DA220);
static auto CL_GetPredictedOriginForServerTime =
    reinterpret_cast<int (*)(clientActive_t *cl, int serverTime, float *predictedOrigin, float *predictedVelocity,
                             int *bobCycle, int *movementDir)>(0x822CAA38);
static auto CL_SetStance = reinterpret_cast<void (*)(int localClientNum, int stance)>(0x822D92A0);

typedef void (*CG_Init_t)(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum);
static CG_Init_t CG_Init = reinterpret_cast<CG_Init_t>(0x8230DEA0);

static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x821D7328);
static auto Scr_AddSourceBuffer =
    reinterpret_cast<char *(*)(const char *filename, const char *extFilename, const char *codePos, bool archive)>(
        0x822212C0);

typedef int (*I_stricmp_t)(const char *s0, const char *s1);
static I_stricmp_t I_stricmp = reinterpret_cast<I_stricmp_t>(0x821CDCC8);

typedef void (*G_AddEvent_t)(gentity_s *ent, int event, int eventParm);
static G_AddEvent_t G_AddEvent = reinterpret_cast<G_AddEvent_t>(0x8224AB48);

typedef int (*Key_StringToKeynum_t)(const char *str);
static Key_StringToKeynum_t Key_StringToKeynum = reinterpret_cast<Key_StringToKeynum_t>(0x822D69A8);

typedef int (*Com_sprintf_t)(char *dest, unsigned int size, const char *fmt, ...);
static Com_sprintf_t Com_sprintf = reinterpret_cast<Com_sprintf_t>(0x821CCED8);

typedef void (*DB_SetXAssetName_t)(XAsset *asset, const char *name);
static DB_SetXAssetName_t DB_SetXAssetName = reinterpret_cast<DB_SetXAssetName_t>(0x822B30D0);

typedef const char *(*DB_GetXAssetName_t)(const XAsset *asset);
static DB_GetXAssetName_t DB_GetXAssetName = reinterpret_cast<DB_GetXAssetName_t>(0x822B3490);

typedef XAssetEntry *(*DB_LinkXAssetEntry_t)(XAssetEntry *newEntry, int allowOverride);
static DB_LinkXAssetEntry_t DB_LinkXAssetEntry = reinterpret_cast<DB_LinkXAssetEntry_t>(0x8229FC50);

typedef void (*DB_LoadXFileData_t)(unsigned __int8 *pos, unsigned int size);
static DB_LoadXFileData_t DB_LoadXFileData = reinterpret_cast<DB_LoadXFileData_t>(0x822B1FC8);

typedef void (*DB_AllocXBlocks_t)(const unsigned int *blockSize, const char *filename, XBlock *blocks,
                                  unsigned int allocType);
static DB_AllocXBlocks_t DB_AllocXBlocks = reinterpret_cast<DB_AllocXBlocks_t>(0x822A1DF0);

typedef void (*Load_XAssetArrayCustom_t)(int count);
static Load_XAssetArrayCustom_t Load_XAssetArrayCustom = reinterpret_cast<Load_XAssetArrayCustom_t>(0x822B2140);

typedef void (*Load_Stream_t)(bool atStreamStart, void *ptr, size_t size);
static Load_Stream_t Load_Stream = reinterpret_cast<Load_Stream_t>(0x8229D148);

typedef void (*Load_DelayStream_t)();
static Load_DelayStream_t Load_DelayStream = reinterpret_cast<Load_DelayStream_t>(0x8229D0F8);

typedef void (*Load_XAssetHeader_t)(bool atStreamStart);
static Load_XAssetHeader_t Load_XAssetHeader = reinterpret_cast<Load_XAssetHeader_t>(0x822B1838);

struct snapshotEntityNumbers_t;

typedef void (*SV_AddEntitiesVisibleFromPoint_t)(const float *org, int clientNum, snapshotEntityNumbers_t *eNums);
static SV_AddEntitiesVisibleFromPoint_t SV_AddEntitiesVisibleFromPoint =
    reinterpret_cast<SV_AddEntitiesVisibleFromPoint_t>(0x821FB898);

typedef void (*DB_LoadXFileInternal_t)();
static DB_LoadXFileInternal_t DB_LoadXFileInternal = reinterpret_cast<DB_LoadXFileInternal_t>(0x822B21B8);

typedef void (*DB_ReadXFileStage_t)();
static DB_ReadXFileStage_t DB_ReadXFileStage = reinterpret_cast<DB_ReadXFileStage_t>(0x822B1F60);

typedef void (*DB_WaitXFileStage_t)();
static DB_WaitXFileStage_t DB_WaitXFileStage = reinterpret_cast<DB_WaitXFileStage_t>(0x822B1DA0);

typedef void (*Com_Error_t)(errorParm_t code, const char *fmt, ...);
static Com_Error_t Com_Error = reinterpret_cast<Com_Error_t>(0x82236640);

typedef void (*R_ShowDirtyDiscError_t)();
static R_ShowDirtyDiscError_t R_ShowDirtyDiscError = reinterpret_cast<R_ShowDirtyDiscError_t>(0x82155218);

typedef int (*DB_AuthLoad_InflateInit_t)(z_stream_s *stream, bool isSecure, const char *filename);
static DB_AuthLoad_InflateInit_t DB_AuthLoad_InflateInit = reinterpret_cast<DB_AuthLoad_InflateInit_t>(0x822B2B90);

typedef void (*Load_XAssetListCustom_t)();
static Load_XAssetListCustom_t Load_XAssetListCustom = reinterpret_cast<Load_XAssetListCustom_t>(0x822B20E0);

typedef void (*DB_PushStreamPos_t)(unsigned int index);
static DB_PushStreamPos_t DB_PushStreamPos = reinterpret_cast<DB_PushStreamPos_t>(0x8229D410);

typedef void (*DB_InitStreams_t)(XBlock *blocks);
static DB_InitStreams_t DB_InitStreams = reinterpret_cast<DB_InitStreams_t>(0x8229D310);

typedef void (*DB_PopStreamPos_t)();
static DB_PopStreamPos_t DB_PopStreamPos = reinterpret_cast<DB_PopStreamPos_t>(0x8229D390);

typedef void (*DB_CancelLoadXFile_t)();
static DB_CancelLoadXFile_t DB_CancelLoadXFile = reinterpret_cast<DB_CancelLoadXFile_t>(0x822B1EC0);

typedef int (*inflateInit2__t)(z_stream_s *z, int w, const char *version, int stream_size);
static inflateInit2__t inflateInit2_ = reinterpret_cast<inflateInit2__t>(0x823527B8);

typedef int (*DB_AuthLoad_Inflate_t)(z_stream_s *stream, int flush);
static DB_AuthLoad_Inflate_t DB_AuthLoad_Inflate = reinterpret_cast<DB_AuthLoad_Inflate_t>(0x822B2F70);

typedef int (*inflate_t)(z_stream_s *stream, int flush);
static inflate_t inflate = reinterpret_cast<inflate_t>(0x82352198);

typedef int (*DB_GetXAssetTypeSize_t)(XAssetType type);
static DB_GetXAssetTypeSize_t DB_GetXAssetTypeSize = reinterpret_cast<DB_GetXAssetTypeSize_t>(0x822B30B8);

// Variables
static auto cgArray = reinterpret_cast<cg_s **>(0x823F28A0);
static auto cgsArray = reinterpret_cast<cgs_t *>(0x823F2890);
static auto clients = reinterpret_cast<clientActive_t **>(0x82435AB8);
static auto clientConnections = reinterpret_cast<clientConnection_t *>(0x824302E0);
static auto clientUIActives = reinterpret_cast<clientUIActive_t *>(0x82435A10);
static auto cg_pmove = reinterpret_cast<pmove_t *>(0x823F6040);
static auto cm = reinterpret_cast<clipMap_t *>(0x82A23240);
static auto cmd_functions = reinterpret_cast<cmd_function_s *>(0x82A2335C);
static auto client_fields = reinterpret_cast<client_fields_s *>(0x82047A18);
static auto dx = reinterpret_cast<DxGlobals *>(0x84CD7A80);
static auto entity_fields = reinterpret_cast<ent_field_t *>(0x82046E00);
static auto g_clients = reinterpret_cast<gclient_s *>(0x829BCD70);
static auto g_entities = reinterpret_cast<gentity_s *>(0x8287CD08);
static ScreenPlacement &scrPlaceFullUnsafe = *reinterpret_cast<ScreenPlacement *>(0x8246F468);
static auto svsHeader = reinterpret_cast<serverStaticHeader_t *>(0x849F1580);
static auto uiInfoArray = reinterpret_cast<uiInfo_s **>(0x849F2DF0);
static PlayerKeyState *playerKeys = reinterpret_cast<PlayerKeyState *>(0x8242AB38);
static unsigned __int8 **g_streamPos = reinterpret_cast<unsigned __int8 **>(0x826B91F4);
static unsigned __int8 **g_streamPosArray = reinterpret_cast<unsigned __int8 **>(0x82708C04);
// XBlock **g_streamBlocks = reinterpret_cast<XBlock **>(0x826AD1EC);
// unsigned __int8 *g_streamPosIndex = reinterpret_cast<unsigned __int8 *>(0x826BA3FC);
static unsigned *g_streamDelayIndex = reinterpret_cast<unsigned *>(0x82668D5C);
static StreamDelayInfo *g_streamDelayArray = reinterpret_cast<StreamDelayInfo *>(0x8270C4F0);
// unsigned __int8 *g_streamPosStackIndex = reinterpret_cast<unsigned __int8 *>(0x82668A34);
static unsigned int *g_loadingAssets = reinterpret_cast<unsigned int *>(0x824754F4);
static bool *g_anyFastFileLoaded = reinterpret_cast<bool *>(0x82435AB5);
static DB_LoadData *g_load = reinterpret_cast<DB_LoadData *>(0x82475508);
static const char **g_block_mem_name = reinterpret_cast<const char **>(0x823A42AC);
static const char **g_assetNames = reinterpret_cast<const char **>(0x823A42C8);
static const char **g_defaultAssetName = reinterpret_cast<const char **>(0x823A40F8);
static const XZoneName *g_zoneNames = reinterpret_cast<XZoneName *>(0x8270BC28);
static const unsigned int *g_zoneIndex = reinterpret_cast<const unsigned int *>(0x82536C4C);
static XAsset **varXAsset = reinterpret_cast<XAsset **>(0x82475658);
static XAssetHeader **varXAssetHeader = reinterpret_cast<XAssetHeader **>(0x824756E0);
static XAssetList **varXAssetList = reinterpret_cast<XAssetList **>(0x824756F4);
static void **DB_XAssetPool = reinterpret_cast<void **>(0x823A4070);
static int *g_poolSize = reinterpret_cast<int *>(0x823A3E50);

static int *g_trackLoadProgress = reinterpret_cast<int *>(0x824754FC);
static int *g_totalSize = reinterpret_cast<int *>(0x824754F8);
static int *g_loadedSize = reinterpret_cast<int *>(0x82475500);
static int *g_totalExternalBytes = reinterpret_cast<int *>(0x82475504);
static int *g_loadedExternalBytes = reinterpret_cast<int *>(0x8246F4AC);

} // namespace mp
} // namespace iw3
