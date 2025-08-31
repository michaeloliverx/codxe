#pragma once
#include "structs.h"

namespace iw5
{
namespace mp
{
static auto cm = reinterpret_cast<clipMap_t *>(0x82FFBE00);

static auto g_assetNames = reinterpret_cast<const char **>(0x825A36A8);
static auto g_zones_0 = reinterpret_cast<XZone *>(0x82C23E24);

static auto g_clients = reinterpret_cast<gclient_s *>(0x82F99580);
static auto g_entities = reinterpret_cast<gentity_s *>(0x82DCCC80);

static auto level = reinterpret_cast<level_locals_t *>(0x82FDA080);

static auto svs_clients = reinterpret_cast<client_t *>(0x836C6310);

static auto CG_GameMessage = reinterpret_cast<CG_GameMessage_t>(0x82127BF8);

static auto DB_FindXAssetEntry = reinterpret_cast<DB_FindXAssetEntry_t>(0x821EE920);
static auto DB_FindXAssetHeader = reinterpret_cast<DB_FindXAssetHeader_t>(0x821EEBF0);
static auto DB_GetXAssetName = reinterpret_cast<DB_GetXAssetName_t>(0x821AB560);
static auto DB_LinkXAssetEntry = reinterpret_cast<DB_LinkXAssetEntry_t>(0x821EF3F0);
static auto DB_IsXAssetDefault = reinterpret_cast<DB_IsXAssetDefault_t>(0x821EEEB0);

static auto PMem_AllocFromSource_NoDebug = reinterpret_cast<PMem_AllocFromSource_NoDebug_t>(0x823335F0);

static auto Scr_AddInt = reinterpret_cast<Scr_AddInt_t>(0x822BFAB8);
static auto GetEntity = reinterpret_cast<GetEntity_t>(0x82251470);
static auto Scr_ErrorInternal = reinterpret_cast<Scr_ErrorInternal_t>(0x822BD368);
static auto Scr_GetInt = reinterpret_cast<Scr_GetInt_t>(0x822BEB30);
static auto Scr_GetMethod = reinterpret_cast<Scr_GetMethod_t>(0x822661A0);
static auto Scr_GetString = reinterpret_cast<Scr_GetString_t>(0x822BF108);

static auto PlayerCmd_GetViewmodel = reinterpret_cast<PlayerCmd_GetViewmodel_t>(0x82233538);
static auto PlayerCmd_UFO = reinterpret_cast<PlayerCmd_UFO_t>(0x82236588);

static auto Weapon_RocketLauncher_Fire = reinterpret_cast<Weapon_RocketLauncher_Fire_t>(0x82271710);

} // namespace mp
} // namespace iw5