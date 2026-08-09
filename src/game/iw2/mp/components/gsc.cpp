#include "pch.h"
#include "gsc.h"
#include "common/gsc_registry.h"

namespace iw2
{
namespace mp
{
// Inlined in the executable, reimplemented here
namespace
{

static unsigned int Scr_GetNumParam()
{
    return *scrVmPub_outparamcount;
}

static unsigned int GScr_AllocString(const char *s)
{
    const char *end = s;
    while (*end++)
        ;

    return SL_GetStringOfLen(s, 1, static_cast<unsigned int>(end - s), 6);
}
} // namespace

void GScr_CbufAddText()
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: exec(<string>)\n");

    const char *text = Scr_GetString(0);
    Cbuf_AddText(text);
}

int CL_IsKeyPressed(const char *keyName)
{
    const int keynum = Key_StringToKeynum(keyName);
    if (keynum >= 0)
        return (*keys)[keynum].down;
    else
        return 0;
}

void PlayerCmd_ButtonPressed(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    const char *button = Scr_GetString(0);

    if (!button || !*button)
        Scr_Error("usage: <client> buttonPressed(<button name>)");

    const int keypressed = CL_IsKeyPressed(button);
    return Scr_AddBool(keypressed);
}

void PlayerCmd_ADSButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddBool(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & CMD_BUTTON_ADS) != 0);
}

void PlayerCmd_JumpButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddBool(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & CMD_BUTTON_JUMP) != 0);
}

void PlayerCmd_FragButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddBool(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & CMD_BUTTON_FRAG) != 0);
}

void PlayerCmd_SmokeButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddBool(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & CMD_BUTTON_SMOKE) != 0);
}

void PlayerCmd_ForwardButtonPressed(scr_entref_t entref)
{
    GetPlayerEntity(entref);
    const client_t *client = &svs->clients[entref.entnum];
    Scr_AddBool(client->lastUsercmd.forwardmove > 0);
}

void PlayerCmd_BackButtonPressed(scr_entref_t entref)
{
    GetPlayerEntity(entref);
    const client_t *client = &svs->clients[entref.entnum];
    Scr_AddBool(client->lastUsercmd.forwardmove < 0);
}

void PlayerCmd_LeftButtonPressed(scr_entref_t entref)
{
    GetPlayerEntity(entref);
    const client_t *client = &svs->clients[entref.entnum];
    Scr_AddBool(client->lastUsercmd.rightmove < 0);
}

void PlayerCmd_RightButtonPressed(scr_entref_t entref)
{
    GetPlayerEntity(entref);
    const client_t *client = &svs->clients[entref.entnum];
    Scr_AddBool(client->lastUsercmd.rightmove > 0);
}

void PlayerCmd_GetStance(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);

    const int pm_flags = ent->client->ps.pm_flags;
    if ((pm_flags & PMF_PRONE) != 0)
    {
        Scr_AddConstString(scr_const->prone);
    }
    else if ((pm_flags & PMF_DUCKED) != 0)
    {
        // 'crouch' isn't part of scr_const.
        static const auto scr_const_crouch = GScr_AllocString("crouch");
        Scr_AddConstString(scr_const_crouch);
    }
    else
    {
        Scr_AddConstString(scr_const->stand);
    }
}

void PlayerCmd_SetStance(scr_entref_t entref)
{
    gentity_s *ent = GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 1)
        Scr_Error("usage: <client> SetStance( <stance> )\n");

    const char *stance = Scr_GetString(0);

    int event = EV_NONE;

    if (!strcmp(stance, "stand"))
        event = EV_STANCE_FORCE_STAND;
    else if (!strcmp(stance, "crouch"))
        event = EV_STANCE_FORCE_CROUCH;
    else if (!strcmp(stance, "prone"))
        event = EV_STANCE_FORCE_PRONE;
    else
    {
        // Scr_ParamError(0, "stance must be 'stand', 'crouch', or 'prone'");
        Scr_Error("stance must be 'stand', 'crouch', or 'prone'");
    }

    G_AddPredictableEvent(ent, event, 0);
}

void PlayerCmd_GetVelocity(scr_entref_t entref)
{
    gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddVector(ent->client->ps.velocity);
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

namespace
{
static const gsc::Entry<BuiltinFunction> functions[] = {
    {"exec", GScr_CbufAddText, BUILTIN_ANY},
};

static const gsc::Entry<BuiltinMethod> methods[] = {
    {"buttonpressed", PlayerCmd_ButtonPressed, BUILTIN_ANY}, // Only works for host buttons
    {"adsbuttonpressed", PlayerCmd_ADSButtonPressed, BUILTIN_ANY},
    {"jumpbuttonpressed", PlayerCmd_JumpButtonPressed, BUILTIN_ANY},
    {"fragbuttonpressed", PlayerCmd_FragButtonPressed, BUILTIN_ANY},
    {"smokebuttonpressed", PlayerCmd_SmokeButtonPressed, BUILTIN_ANY},
    {"forwardbuttonpressed", PlayerCmd_ForwardButtonPressed, BUILTIN_ANY},
    {"backbuttonpressed", PlayerCmd_BackButtonPressed, BUILTIN_ANY},
    {"leftbuttonpressed", PlayerCmd_LeftButtonPressed, BUILTIN_ANY},
    {"rightbuttonpressed", PlayerCmd_RightButtonPressed, BUILTIN_ANY},
    {"getstance", PlayerCmd_GetStance, BUILTIN_ANY},
    {"setstance", PlayerCmd_SetStance, BUILTIN_ANY},
    {"getvelocity", PlayerCmd_GetVelocity, BUILTIN_ANY},
    {"setvelocity", PlayerCmd_SetVelocity, BUILTIN_ANY},
};
} // namespace

Detour Scr_GetFunction_Detour;

BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
{
    if (pName)
    {
        const gsc::Entry<BuiltinFunction> *function = gsc::Find(*pName, functions);
        if (function)
        {
            *type = function->type;
            return function->actionFunc;
        }
    }

    return Scr_GetFunction_Detour.GetOriginal<decltype(Scr_GetFunction)>()(pName, type);
}

Detour Scr_GetMethod_Detour;

BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
{
    if (pName)
    {
        const gsc::Entry<BuiltinMethod> *method = gsc::Find(*pName, methods);
        if (method)
        {
            *type = method->type;
            return method->actionFunc;
        }
    }

    return Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
}

GSC::GSC()
{
    Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
    Scr_GetFunction_Detour.Install();

    Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
    Scr_GetMethod_Detour.Install();
}

GSC::~GSC()
{
    Scr_GetFunction_Detour.Remove();

    Scr_GetMethod_Detour.Remove();
}
} // namespace mp
} // namespace iw2
