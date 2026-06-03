#pragma once

#include "structs.h"

namespace iw4
{

namespace mp
{
typedef const ScreenPlacement *(*ScrPlace_GetActivePlacement_t)(int localClientNum);

typedef void (*Com_InitDvars_t)();

typedef dvar_t *(*Dvar_RegisterBool_t)(const char *dvarName, bool value, std::uint16_t flags, const char *description);

typedef dvar_t *(*Dvar_RegisterFloat_t)(const char *dvarName, double value, double min, double max, std::uint16_t flags,
                                        const char *description);

typedef const char *(*Dvar_GetString_t)(const char *dvarName);

typedef void (*UI_DrawBuildNumber_t)(int localClientNum);

typedef void (*UI_DrawText_t)(const ScreenPlacement *scrPlace, const char *text, int maxChars, Font_s *font, double x,
                              double y, int horzAlign, int vertAlign, double scale, const float *color, int style);

typedef gentity_s *(*Weapon_RocketLauncher_Fire_t)(gentity_s *ent, unsigned int weaponIndex, double spread,
                                                   weaponParms *wp, weaponParms *gunVel, lockonFireParms *lockParms,
                                                   lockonFireParms *magicBullet, bool magicBulletEnabled);

static auto ScrPlace_GetActivePlacement = reinterpret_cast<ScrPlace_GetActivePlacement_t>(0x82165410);

static auto Com_InitDvars = reinterpret_cast<Com_InitDvars_t>(0x8222D0A8);

static auto Dvar_RegisterBool = reinterpret_cast<Dvar_RegisterBool_t>(0x822A0060);
static auto Dvar_RegisterFloat = reinterpret_cast<Dvar_RegisterFloat_t>(0x822A00E8);
static auto Dvar_GetString = reinterpret_cast<Dvar_GetString_t>(0x8229F0A8);

static auto Weapon_RocketLauncher_Fire = reinterpret_cast<Weapon_RocketLauncher_Fire_t>(0x8220FE70);

static auto UI_DrawBuildNumber = reinterpret_cast<UI_DrawBuildNumber_t>(0x8226DA18);
static auto UI_DrawText = reinterpret_cast<UI_DrawText_t>(0x8226D420);

static auto sharedUiInfo = reinterpret_cast<sharedUiInfo_t *>(0x8364D440);
} // namespace mp
} // namespace iw4
