#pragma once

#include "structs.h"

namespace iw2
{
namespace mp
{
static const unsigned int *scrVmPub_outparamcount = reinterpret_cast<const unsigned int *>(0x8447399C);

static auto g_entities = reinterpret_cast<gentity_s *>(0x83676880);
static auto level = reinterpret_cast<level_locals_t *>(0x837E4A00);
static auto svs = reinterpret_cast<serverStatic_t *>(0x84517300);
static const dvar_s **sv_maxclients = reinterpret_cast<const dvar_s **>(0x8446EF88);
static qkey_t **keys = reinterpret_cast<qkey_t **>(0x824D3518);

static auto player_methods = reinterpret_cast<BuiltinMethodDef *>(0x8203EAC8);
static const int PLAYER_METHODS_COUNT = 59;

static const scr_const_t *scr_const = reinterpret_cast<scr_const_t *>(0x8358EF08);

typedef dvar_s *(*Dvar_RegisterVariant_t)(const char *dvarName, DvarType type, DvarFlags flags, unsigned int value,
                                          unsigned int domain);
static Dvar_RegisterVariant_t Dvar_RegisterVariant = reinterpret_cast<Dvar_RegisterVariant_t>(0x8230BFB0);

typedef void (*Jump_RegisterDvars_t)();
static Jump_RegisterDvars_t Jump_RegisterDvars = reinterpret_cast<Jump_RegisterDvars_t>(0x8246AFB8);

typedef void (*Cbuf_AddText_t)(const char *text);
static Cbuf_AddText_t Cbuf_AddText = reinterpret_cast<Cbuf_AddText_t>(0x82370100);

typedef void *(*Hunk_AllocateTempMemoryHighInternal_t)(int size);
static Hunk_AllocateTempMemoryHighInternal_t Hunk_AllocateTempMemoryHighInternal =
    reinterpret_cast<Hunk_AllocateTempMemoryHighInternal_t>(0x8230DFB8);

typedef char *(*Scr_AddSourceBuffer_t)(const char *filename, const char *extFilename, const char *codePos,
                                       bool archive);
static Scr_AddSourceBuffer_t Scr_AddSourceBuffer = reinterpret_cast<Scr_AddSourceBuffer_t>(0x82354250);

typedef char *(*Scr_GetString_t)(unsigned int index);
static Scr_GetString_t Scr_GetString = reinterpret_cast<Scr_GetString_t>(0x82345418);

typedef gentity_s *(*GetPlayerEntity_t)(scr_entref_t entref);
static GetPlayerEntity_t GetPlayerEntity = reinterpret_cast<GetPlayerEntity_t>(0x823CCC28);

typedef void (*G_AddPredictableEvent_t)(gentity_s *ent, int event, unsigned __int8 eventParm);
static G_AddPredictableEvent_t G_AddPredictableEvent = reinterpret_cast<G_AddPredictableEvent_t>(0x823C5DB0);

typedef void (*Scr_Error_t)(const char *error);
static auto Scr_Error = reinterpret_cast<Scr_Error_t>(0x82342AC0);

typedef void (*Scr_AddBool_t)(int value);
static auto Scr_AddBool = reinterpret_cast<Scr_AddBool_t>(0x82346948);

typedef void (*Scr_AddInt_t)(int value);
static auto Scr_AddInt = reinterpret_cast<Scr_AddInt_t>(0x82346900);

typedef int (*Scr_GetInt_t)(unsigned int index);
static Scr_GetInt_t Scr_GetInt = reinterpret_cast<Scr_GetInt_t>(0x82343258);

typedef void (*Scr_AddString_t)(const char *value);
static Scr_AddString_t Scr_AddString = reinterpret_cast<Scr_AddString_t>(0x82346748);

typedef void (*Scr_AddEntityNum_t)(unsigned int entnum, unsigned int classnum);
static Scr_AddEntityNum_t Scr_AddEntityNum = reinterpret_cast<Scr_AddEntityNum_t>(0x823469F0);

typedef gentity_s *(*SV_AddTestClient_t)();
static SV_AddTestClient_t SV_AddTestClient = reinterpret_cast<SV_AddTestClient_t>(0x8233CC70);

typedef const char *(*ClientConnect_t)(int clientNum, unsigned int scriptId);
static ClientConnect_t ClientConnect = reinterpret_cast<ClientConnect_t>(0x823EC5C0);

typedef void (*Info_SetValueForKey_t)(char *info, const char *key, const char *value);
static Info_SetValueForKey_t Info_SetValueForKey = reinterpret_cast<Info_SetValueForKey_t>(0x82303DC8);

typedef void (*SV_BotUserMove_t)(client_t *client);
static SV_BotUserMove_t SV_BotUserMove = reinterpret_cast<SV_BotUserMove_t>(0x823367F8);

typedef void (*SV_ClientThink_t)(client_t *client, const usercmd_s *cmd);
static SV_ClientThink_t SV_ClientThink = reinterpret_cast<SV_ClientThink_t>(0x8233D0D8);

typedef void (*SV_ClientEnterWorld_t)(client_t *client, const usercmd_s *cmd);
static SV_ClientEnterWorld_t SV_ClientEnterWorld = reinterpret_cast<SV_ClientEnterWorld_t>(0x8233BAE8);

typedef void (*SV_CalcPings_t)();
static SV_CalcPings_t SV_CalcPings = reinterpret_cast<SV_CalcPings_t>(0x82335260);

typedef void (*SV_GameSendServerCommand_t)(int clientNum, int type, const char *text);
static SV_GameSendServerCommand_t SV_GameSendServerCommand = reinterpret_cast<SV_GameSendServerCommand_t>(0x8233A1F0);

typedef int (*BG_IsWeaponValid_t)(const playerState_s *ps, int weaponIndex);
static BG_IsWeaponValid_t BG_IsWeaponValid = reinterpret_cast<BG_IsWeaponValid_t>(0x8245A398);

typedef void (*Scr_ShutdownSystem_t)(int complete, int freeScripts);
static Scr_ShutdownSystem_t Scr_ShutdownSystem = reinterpret_cast<Scr_ShutdownSystem_t>(0x82346250);

inline unsigned int Scr_GetNumParam()
{
    return *scrVmPub_outparamcount;
}

typedef void (*Scr_AddConstString_t)(int value);
static auto Scr_AddConstString = reinterpret_cast<Scr_AddConstString_t>(0x82346648);

typedef void (*Scr_GetVector_t)(unsigned int index, float *vectorValue);
static Scr_GetVector_t Scr_GetVector = reinterpret_cast<Scr_GetVector_t>(0x82342FB0);

typedef void (*Scr_AddVector_t)(float *value);
static Scr_AddVector_t Scr_AddVector = reinterpret_cast<Scr_AddVector_t>(0x823465F8);

typedef int (*Key_StringToKeynum_t)(const char *str);
static Key_StringToKeynum_t Key_StringToKeynum = reinterpret_cast<Key_StringToKeynum_t>(0x8241E338);

typedef BuiltinFunction (*Scr_GetFunction_t)(const char **pName, int *type);
static Scr_GetFunction_t Scr_GetFunction = reinterpret_cast<Scr_GetFunction_t>(0x823CB720);

typedef BuiltinMethod (*Scr_GetMethod_t)(const char **pName, int *type);
static Scr_GetMethod_t Scr_GetMethod = reinterpret_cast<Scr_GetMethod_t>(0x823CB968);

typedef unsigned int (*SL_GetStringOfLen_t)(const char *str, unsigned int user, unsigned int len, int type);
static auto SL_GetStringOfLen = reinterpret_cast<SL_GetStringOfLen_t>(0x82351F58);

typedef char *(*va_t)(char *format, ...);
static va_t va = reinterpret_cast<va_t>(0x82304090);
} // namespace mp
} // namespace iw2
