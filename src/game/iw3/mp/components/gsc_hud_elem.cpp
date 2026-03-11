/**
 * Straight from cod4x.
 */
#include "pch.h"

#include "events.h"
#include "gsc_hud_elem.h"

namespace iw3
{
namespace mp
{

static bool g_isLocStringPrecached[MAX_LOCALIZEDSTRINGS] = {};

bool Scr_CanFreeLocalizedConfigString(unsigned int index)
{
    /* Index not set + fast return from function */
    if (!index)
        return false;

    /* Overflow protection */
    if (index >= MAX_CONFIGSTRINGS)
    {
        Scr_Error(va("localized configstring index must be between 0 and %d", MAX_CONFIGSTRINGS - 1));
        return false;
    }

    /* Better not to free precached strings... + fast return */
    if (g_isLocStringPrecached[index] == true)
        return false;

    /* Check all script hud elements if index in use SLOOOOW :C */
    for (int i = 0; i < 1024; ++i)
    {
        game_hudelem_s *elem = &g_hudelems[i];
        if (elem->elem.text && (unsigned int)elem->elem.text == index)
            return false;
    }

    return true;
}

Detour Scr_PrecacheString_Detour;
void Scr_PrecacheString_Stub()
{
    DbgPrint("[Scr_PrecacheString]\n");

    if (!level->initializing)
    {
        Scr_Error("precacheString must be called before any wait statements in the gametype or level script\n");
    }

    const char *locStrName = Scr_GetIString(0);
    if (locStrName[0])
        g_isLocStringPrecached[G_LocalizedStringIndex(locStrName)] = true;
}

static game_hudelem_s *HECmd_GetHudElem(scr_entref_t entref)
{
    if (entref.classnum == CLASS_NUM_HUDELEM)
    {
        return &g_hudelems[entref.entnum];
    }
    else
    {
        Scr_ObjectError("not a hud element");
        return 0;
    }
}

Detour HECmd_SetText_Detour;

void HECmd_SetText_Stub(scr_entref_t entref)
{
    game_hudelem_s *hud = HECmd_GetHudElem(entref);

    const int cs_index = hud->elem.text;

    HudElem_ClearTypeSettings(hud);

    /* Attempt to avoid CS overflow using "SetText()" */
    if (Scr_CanFreeLocalizedConfigString(cs_index))
        SV_SetConfigstring(cs_index + CS_LOCALIZEDSTRINGS, "");

    char buffer[1024];
    Scr_ConstructMessageString(0, 0, "Hud Elem String", buffer, 1024);

    hud->elem.type = HE_TYPE_TEXT;
    hud->elem.text = G_LocalizedStringIndex(buffer);
}

Detour Scr_FreeHudElem_Detour;
void Scr_FreeHudElem_Hook(game_hudelem_s *hud)
{
    DbgPrint("[Scr_FreeHudElem]\n");
    int32_t cs_index = hud->elem.text;
    hud->elem.text = 0;

    if (Scr_CanFreeLocalizedConfigString(cs_index))
        SV_SetConfigstring(cs_index + CS_LOCALIZEDSTRINGS, "");

    Scr_FreeHudElem_Detour.GetOriginal<Scr_FreeHudElem_t>()(hud);
}

gsc_hud_elem::gsc_hud_elem()
{
    Scr_PrecacheString_Detour = Detour(Scr_PrecacheString, Scr_PrecacheString_Stub);
    Scr_PrecacheString_Detour.Install();

    HECmd_SetText_Detour = Detour(HECmd_SetText, HECmd_SetText_Stub);
    HECmd_SetText_Detour.Install();

    Scr_FreeHudElem_Detour = Detour(Scr_FreeHudElem, Scr_FreeHudElem_Hook);
    Scr_FreeHudElem_Detour.Install();

    Events::OnVMShutdown(
        []()
        {
            //
            memset(g_isLocStringPrecached, 0, sizeof(g_isLocStringPrecached));
        });
}

gsc_hud_elem::~gsc_hud_elem()
{
    Scr_PrecacheString_Detour.Remove();
    HECmd_SetText_Detour.Remove();
    Scr_FreeHudElem_Detour.Remove();
}
} // namespace mp
} // namespace iw3
