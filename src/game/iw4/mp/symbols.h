#pragma once

#include "structs.h"

namespace iw4
{
namespace mp
{
// Functions
static auto Hunk_AllocateTempMemoryHighInternal = reinterpret_cast<void *(*)(int size)>(0x822FDF08);

static auto Cbuf_AddText = reinterpret_cast<void (*)(int localClientNum, const char *text)>(0x82275C60);

static auto CG_GameMessage = reinterpret_cast<void (*)(int localClientNum, const char *msg)>(0x8213DE38);

static auto Cmd_AddCommandInternal =
    reinterpret_cast<void (*)(const char *cmdName, void (*function)(), cmd_function_s *allocedCmd)>(0x82276758);

static auto DB_LinkXAssetEntry1 =
    reinterpret_cast<XAssetEntryPoolEntry *(*)(XAssetType type, XAssetHeader *header)>(0x821DE528);

static auto Scr_AddSourceBuffer =
    reinterpret_cast<char *(*)(const char *filename, const char *extFilename)>(0x8229F2C8);

static auto Scr_AddClassField =
    reinterpret_cast<void (*)(unsigned int classnum, const char *name, unsigned __int16 offset)>(0x822A9B98);
static auto GScr_AddFieldsForClient = reinterpret_cast<void (*)()>(0x8221B238);
static auto Scr_SetClientField = reinterpret_cast<void (*)(gclient_s *client, int offset)>(0x8221B290);
static auto Scr_SetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs)>(0x8225A698);
static auto Scr_GetGenericField = reinterpret_cast<void (*)(unsigned __int8 *b, fieldtype_t type, int ofs)>(0x8225A7B8);
static auto Scr_GetObjectField = reinterpret_cast<void (*)(unsigned int classnum, int entnum, int offset)>(0x8225ABF0);

static auto Scr_GetString = reinterpret_cast<const char *(*)(unsigned int index)>(0x822B33A8);
static auto Scr_AddInt = reinterpret_cast<void (*)(int value)>(0x822ADD18);
static auto Scr_GetInt = reinterpret_cast<int (*)(unsigned int index)>(0x822B2D70);
static auto Scr_GetVector = reinterpret_cast<void (*)(unsigned int index, float *vectorValue)>(0x822B35B8);
static auto Scr_Error = reinterpret_cast<void (*)(const char *error)>(0x822AE470);
static auto GetEntity = reinterpret_cast<gentity_s *(*)(scr_entref_t entref)>(0x8223F4D0);
static auto Scr_ObjectError = reinterpret_cast<void (*)(const char *error)>(0x822AE668);

static auto Scr_RegisterFunction = reinterpret_cast<void (*)(int func, const char *name)>(0x822963C0);

static auto Scr_GetFunction = reinterpret_cast<BuiltinFunction (*)(const char **pName, int *type)>(0x82254C38);
static auto Scr_GetMethod = reinterpret_cast<BuiltinMethod (*)(const char **pName, int *type)>(0x82254D88);

static auto Scr_GetNumParam = reinterpret_cast<unsigned int (*)()>(0x822ADC88);

static auto Weapon_RocketLauncher_Fire =
    reinterpret_cast<gentity_s *(*)(gentity_s * ent, unsigned int weaponIndex, double spread, weaponParms *wp,
                                    weaponParms *gunVel, struct lockonFireParms *lockParms,
                                    lockonFireParms *magicBullet)>(0x82260C90);
static auto ScrPlace_GetUnsafeFullPlacement = reinterpret_cast<const ScreenPlacement *(*)()>(0x821AB7A0);

static auto SCR_DrawScreenField = reinterpret_cast<void (*)(int localClientNum, int refreshedUI)>(0x8218E800);

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

static auto CG_GetPredictedPlayerState = reinterpret_cast<playerState_s *(*)(int localClientNum)>(0x8213DE18);
static auto CL_CreateNewCommands = reinterpret_cast<void (*)(int localClientNum)>(0x8217E540);

static auto cm = reinterpret_cast<clipMap_t *>(0x83052680);
static auto fields = reinterpret_cast<client_fields_s *>(0x82027518);
static auto g_entities = reinterpret_cast<gentity_s *>(0x82E2A580);
static auto level = reinterpret_cast<level_locals_t *>(0x82FF2F08);
static auto sharedUiInfo = reinterpret_cast<sharedUiInfo_t *>(0x836A3AC0);

} // namespace mp
} // namespace iw4
