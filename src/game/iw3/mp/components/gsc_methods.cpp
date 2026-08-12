// cod4x

#include "pch.h"
#include "gsc_methods.h"

namespace iw3
{
namespace mp
{
namespace
{
/**
 * Straight from cod4x.
 */
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

game_hudelem_s *HECmd_GetHudElem(scr_entref_t entref)
{
    if (entref.classnum == CLASS_NUM_HUDELEM)
    {
        return &g_hudelems[entref.entnum];
    }

    Scr_ObjectError("not a hud element");
    return 0;
}

Detour Scr_FreeHudElem_Detour;

void Scr_FreeHudElem_Hook(game_hudelem_s *hud)
{
    int32_t cs_index = hud->elem.text;
    hud->elem.text = 0;

    if (Scr_CanFreeLocalizedConfigString(cs_index))
        SV_SetConfigstring(cs_index + CS_LOCALIZEDSTRINGS, "");

    Scr_FreeHudElem_Detour.GetOriginal<Scr_FreeHudElem_t>()(hud);
}
} // namespace

void Scr_PrecacheString_Stub()
{
    if (!level->initializing)
    {
        Scr_Error("precacheString must be called before any wait statements in the gametype or level script\n");
    }

    const char *locStrName = Scr_GetIString(0);
    if (locStrName[0])
        g_isLocStringPrecached[G_LocalizedStringIndex(locStrName)] = true;
}

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

void InitializeHudElemMethods()
{
    Scr_FreeHudElem_Detour = Detour(Scr_FreeHudElem, Scr_FreeHudElem_Hook);
    Scr_FreeHudElem_Detour.Install();
}

void ShutdownHudElemMethods()
{
    Scr_FreeHudElem_Detour.Remove();
}

void ClearHudElemLocalizedStringState()
{
    ZeroMemory(g_isLocStringPrecached, sizeof(g_isLocStringPrecached));
}

int CL_IsKeyPressed(const int localClientNum, const char *keyName)
{
    const int keynum = Key_StringToKeynum(keyName);
    if (keynum >= 0)
        return playerKeys[0].keys[keynum].down;
    else
        return 0;
}

void PlayerCmd_ButtonPressed(scr_entref_t entref)
{
    if (entref.classnum != 0)
        Scr_ObjectError("not an entity");

    const char *button = Scr_GetString(0);
    if (!button || !*button)
        Scr_Error("usage: <client> buttonPressed(<button name>)");

    const int keypressed = CL_IsKeyPressed(0, button);
    return Scr_AddInt(keypressed);
}

void PlayerCmd_SprintButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_MASK_SPRINT) != 0);
}

void PlayerCmd_LeanLeftButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_MASK_LEANLEFT) != 0);
}

void PlayerCmd_LeanRightButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_MASK_LEANRIGHT) != 0);
}

void PlayerCmd_JumpButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_MASK_JUMP) != 0);
}

void PlayerCmd_HoldBreathButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_MASK_HOLDBREATH) != 0);
}

void PlayerCmd_NightVisionButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_MASK_NIGHTVISION) != 0);
}

void PlayerCmd_ForwardButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    const client_t *cl = &svsHeader->clients[ent->s.number];

    Scr_AddInt(cl->lastUsercmd.forwardmove > 0);
}

void PlayerCmd_BackButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    const client_t *cl = &svsHeader->clients[ent->s.number];

    Scr_AddInt(cl->lastUsercmd.forwardmove < 0);
}

void PlayerCmd_LeftButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    const client_t *cl = &svsHeader->clients[ent->s.number];

    Scr_AddInt(cl->lastUsercmd.rightmove < 0);
}

void PlayerCmd_RightButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    const client_t *cl = &svsHeader->clients[ent->s.number];

    Scr_AddInt(cl->lastUsercmd.rightmove > 0);
}

void PlayerCmd_SetVelocity(scr_entref_t entref)
{
    gentity_s *ent = GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: <client> SetVelocity( vec3 )\n");

    float velocity[3] = {0};

    Scr_GetVector(0, velocity);

    ent->client->ps.velocity[0] = velocity[0];
    ent->client->ps.velocity[1] = velocity[1];
    ent->client->ps.velocity[2] = velocity[2];
}

void PlayerCmd_SetStance(scr_entref_t entref)
{
    gentity_s *ent = GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 1)
        Scr_Error("usage: <client> SetStance( <stance> )\n");

    const char *stanceStr = Scr_GetString(0);

    int event = -1;
    int newPmFlags = ent->client->ps.pm_flags;

    if (!I_stricmp(stanceStr, "stand"))
    {
        event = EV_STANCE_FORCE_STAND;
        newPmFlags = (newPmFlags & ~0x3) | CL_STANCE_STAND;
    }
    else if (!I_stricmp(stanceStr, "crouch"))
    {
        event = EV_STANCE_FORCE_CROUCH;
        newPmFlags = (newPmFlags & ~0x3) | CL_STANCE_CROUCH;
    }
    else if (!I_stricmp(stanceStr, "prone"))
    {
        event = EV_STANCE_FORCE_PRONE;
        newPmFlags = (newPmFlags & ~0x3) | CL_STANCE_PRONE;
    }
    else
    {
        Scr_ParamError(0, "stance must be 'stand', 'crouch', or 'prone'");
    }

    // Update server-side stance flags
    ent->client->ps.pm_flags = newPmFlags;

    // Send event to client to sync stance visually
    G_AddEvent(ent, event, 0);
}

void GScr_CloneBrushModelToScriptModel(scr_entref_t entref)
{
    gentity_s *scriptEnt = GetEntity(entref);
    gentity_s *brushEnt = Scr_GetEntity(0);

    SV_UnlinkEntity(scriptEnt);
    scriptEnt->s.index = brushEnt->s.index;
    int contents = scriptEnt->r.contents;
    SV_SetBrushModel(scriptEnt);
    scriptEnt->r.contents |= contents;
    SV_LinkEntity(scriptEnt);
}

void GScr_SetBrushModel(scr_entref_t entref)
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("usage: <entity> SetBrushModel( <index> )\n");

    gentity_s *ent = GetEntity(entref);
    const int index = Scr_GetInt(0);

    if (index < 0 || (unsigned int)index >= cm->numSubModels)
    {
        Scr_ParamError(0, "brush model index out of range");
    }

    SV_UnlinkEntity(ent);
    ent->s.index = index;

    SV_SetBrushModel(ent);
    SV_LinkEntity(ent);
}
} // namespace mp
} // namespace iw3
