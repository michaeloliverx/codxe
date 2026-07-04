#pragma once
#include "structs.h"

namespace iw5
{
namespace mp
{
// Global variables

static auto cm = reinterpret_cast<clipMap_t *>(0x82FFBE00);

static auto g_assetNames = reinterpret_cast<const char **>(0x825A36A8);
static auto g_zones_0 = reinterpret_cast<XZone *>(0x82C23E24);

static auto g_clients = reinterpret_cast<gclient_s *>(0x82F99580);
static auto g_entities = reinterpret_cast<gentity_s *>(0x82DCCC80);

static auto level = reinterpret_cast<level_locals_t *>(0x82FDA080);

static auto sharedUiInfo = reinterpret_cast<sharedUiInfo_t *>(0x836EE5B0);

static auto svs_numclients = reinterpret_cast<int *>(0x836C630C);
static auto svs_clients = reinterpret_cast<client_t **>(0x836C6310);

// Function pointers

static auto CG_GameMessage = reinterpret_cast<CG_GameMessage_t>(0x82127BF8);

static auto CL_RegisterFont = reinterpret_cast<CL_RegisterFont_t>(0x82174F88);

static auto DB_FindXAssetEntry = reinterpret_cast<DB_FindXAssetEntry_t>(0x821EE920);
static auto DB_FindXAssetHeader = reinterpret_cast<DB_FindXAssetHeader_t>(0x821EEBF0);
static auto DB_GetXAssetName = reinterpret_cast<DB_GetXAssetName_t>(0x821AB560);
static auto DB_LinkXAssetEntry = reinterpret_cast<DB_LinkXAssetEntry_t>(0x821EF3F0);
static auto DB_IsXAssetDefault = reinterpret_cast<DB_IsXAssetDefault_t>(0x821EEEB0);

static auto Dvar_FindMalleableVar = reinterpret_cast<Dvar_FindMalleableVar_t>(0x8232E100);

static auto Jump_Start = reinterpret_cast<Jump_Start_t>(0x820DAF00);

static auto PMem_AllocFromSource_NoDebug = reinterpret_cast<PMem_AllocFromSource_NoDebug_t>(0x823335F0);

static auto Scr_AddInt = reinterpret_cast<Scr_AddInt_t>(0x822BFAB8);
static auto GetEntity = reinterpret_cast<GetEntity_t>(0x82251470);
static auto Scr_ErrorInternal = reinterpret_cast<Scr_ErrorInternal_t>(0x822BD368);
static auto Scr_GetInt = reinterpret_cast<Scr_GetInt_t>(0x822BEB30);
static auto Scr_GetMethod = reinterpret_cast<Scr_GetMethod_t>(0x822661A0);
static auto Scr_GetString = reinterpret_cast<Scr_GetString_t>(0x822BF108);
static auto SL_GetString = reinterpret_cast<Sl_GetString_t>(0x822B5CC8);
static auto Scr_GetNumParam = reinterpret_cast<Scr_GetNumParam_t>(0x822BFA70);
static auto Scr_AddString = reinterpret_cast<void (*)(char *value)>(0x822BFC68);
static auto Scr_NotifyNum =
    reinterpret_cast<void (*)(int entnum, unsigned int classnum, unsigned int stringValue, unsigned int paramcount)>(
        0x822BE548);

static auto ScrPlace_GetActivePlacement = reinterpret_cast<ScrPlace_GetActivePlacement_t>(0x821A69E8);

static auto PlayerCmd_GetViewmodel = reinterpret_cast<PlayerCmd_GetViewmodel_t>(0x82233538);
static auto PlayerCmd_UFO = reinterpret_cast<PlayerCmd_UFO_t>(0x82236588);

static auto va = reinterpret_cast<char *(*)(const char *format, ...)>(0x823365F0);

static auto UI_DrawBuildNumber = reinterpret_cast<UI_DrawBuildNumber_t>(0x822ECA88);
static auto UI_DrawText = reinterpret_cast<UI_DrawText_t>(0x822EC490);

static auto Weapon_RocketLauncher_Fire = reinterpret_cast<Weapon_RocketLauncher_Fire_t>(0x82271710);

static auto G_IRand = reinterpret_cast<int (*)(int min, int max)>(0x8226FC80);
static auto GetProtocolVersion = reinterpret_cast<int (*)()>(0x8232A320);
static auto BG_NetDataChecksum = reinterpret_cast<int (*)()>(0x820E0B50);
static auto SV_Cmd_TokenizeString = reinterpret_cast<void (*)(const char *text)>(0x822889F0);
static auto SV_DirectConnect = reinterpret_cast<void (*)(netadr_t from, int client)>(0x822C8AB0);
static auto SV_Cmd_EndTokenizedString = reinterpret_cast<void (*)()>(0x82288A10);
static auto SV_SendClientGameState = reinterpret_cast<void (*)(client_t *cl)>(0x822C6DD0);
static auto SV_ClientEnterWorld = reinterpret_cast<void (*)(client_t *cl, usercmd_s *cmd)>(0x822C6F50);

static auto SV_DropClient = reinterpret_cast<SV_DropClient_t>(0x822C66A8);


} // namespace mp
} // namespace iw5