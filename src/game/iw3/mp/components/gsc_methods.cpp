// cod4x

#include "pch.h"
#include "g_scr_main.h"
#include "gsc_methods.h"

namespace iw3
{
namespace mp
{

// #define KEY_MASK_FIRE           1
#define KEY_MASK_SPRINT 2
// #define KEY_MASK_MELEE          4
// #define KEY_MASK_RELOAD         16
#define KEY_MASK_LEANLEFT 64
#define KEY_MASK_LEANRIGHT 128
// #define KEY_MASK_PRONE          256
// #define KEY_MASK_CROUCH         512
#define KEY_MASK_JUMP 1024
// #define KEY_MASK_ADS_MODE       2048
// #define KEY_MASK_TEMP_ACTION    4096
#define KEY_MASK_HOLDBREATH 8192
// #define KEY_MASK_FRAG           16384
// #define KEY_MASK_SMOKE          32768
#define KEY_MASK_NIGHTVISION 262144
// #define KEY_MASK_ADS            524288
// #define KEY_MASK_USE            8
// #define KEY_MASK_USERELOAD      0x20
// #define BUTTON_ATTACK			KEY_MASK_FIRE

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

    char *button = Scr_GetString(0);
    if (!button || !*button)
        Scr_Error("usage: <client> buttonPressed(<button name>)");

    // toupper
    for (char *p = button; *p; p++)
        if (*p >= 'a' && *p <= 'z')
            *p -= 32;

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

gsc_methods::gsc_methods()
{
    // Player entity methods
    Scr_AddMethod("buttonpressed", PlayerCmd_ButtonPressed, 0); // Host-only
    Scr_AddMethod("sprintbreathbuttonpressed", PlayerCmd_SprintButtonPressed, 0);
    Scr_AddMethod("leanleftbuttonpressed", PlayerCmd_LeanLeftButtonPressed, 0);
    Scr_AddMethod("leanrightbuttonpressed", PlayerCmd_LeanRightButtonPressed, 0);
    Scr_AddMethod("jumpbuttonpressed", PlayerCmd_JumpButtonPressed, 0);
    Scr_AddMethod("holdbreathbuttonpressed", PlayerCmd_HoldBreathButtonPressed, 0);
    Scr_AddMethod("nightvisionbuttonpressed", PlayerCmd_NightVisionButtonPressed, 0);
    Scr_AddMethod("forwardbuttonpressed", PlayerCmd_ForwardButtonPressed, 0);
    Scr_AddMethod("backbuttonpressed", PlayerCmd_BackButtonPressed, 0);
    Scr_AddMethod("leftbuttonpressed", PlayerCmd_LeftButtonPressed, 0);
    Scr_AddMethod("rightbuttonpressed", PlayerCmd_RightButtonPressed, 0);
    Scr_AddMethod("setvelocity", PlayerCmd_SetVelocity, 0);
    Scr_AddMethod("setstance", PlayerCmd_SetStance, 0);

    // Script entity methods
    Scr_AddMethod("clonebrushmodeltoscriptmodel", GScr_CloneBrushModelToScriptModel, 0);
    Scr_AddMethod("setbrushmodel", GScr_SetBrushModel, 0);
}

gsc_methods::~gsc_methods()
{
}
} // namespace mp
} // namespace iw3
