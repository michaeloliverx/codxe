#include "pch.h"
#include "gsc.h"
#include "common/gsc_registry.h"
#include "clipmap.h"

namespace qos
{
namespace mp
{
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
    GetPlayerEntity(entref);

    const char *button = Scr_GetString(0);
    if (!button || !*button)
        Scr_Error("usage: <client> buttonPressed(<button name>)");

    const int keypressed = CL_IsKeyPressed(0, button);
    return Scr_AddInt(keypressed);
}

void PlayerCmd_ADSButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & CMD_BUTTON_ADS) != 0);
}

void PlayerCmd_JumpButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & CMD_BUTTON_JUMP) != 0);
}

void PlayerCmd_NextFireTypeButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & CMD_BUTTON_NEXTFIRETYPE) != 0);
}

void PlayerCmd_SprintButtonPressed(scr_entref_t entref)
{
    const gentity_s *ent = GetPlayerEntity(entref);
    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & CMD_BUTTON_SPRINT) != 0);
}

void PlayerCmd_SetPlayerVelocity(scr_entref_t entref)
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

void GScr_CbufAddText()
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: exec(<string>)\n");

    const char *text = Scr_GetString(0);
    Cbuf_AddText(0, text);
}

namespace
{
static const BuiltinFunctionDef functions[] = {
    {"exec", GScr_CbufAddText, BUILTIN_ANY},
};

static const BuiltinMethodDef methods[] = {
    {"disableplayerclipontouchingbrushes", clipmap::PlayerCmd_DisablePlayerClipOnTouchingBrushes, BUILTIN_ANY},
    {"buttonpressed", PlayerCmd_ButtonPressed, BUILTIN_ANY}, // Only works for host buttons
    {"adsbuttonpressed", PlayerCmd_ADSButtonPressed, BUILTIN_ANY},
    {"jumpbuttonpressed", PlayerCmd_JumpButtonPressed, BUILTIN_ANY},
    {"nextfiretypebuttonpressed", PlayerCmd_NextFireTypeButtonPressed, BUILTIN_ANY},
    {"sprintbuttonpressed", PlayerCmd_SprintButtonPressed, BUILTIN_ANY},
    {"setplayervelocity", PlayerCmd_SetPlayerVelocity, BUILTIN_ANY},
};
} // namespace

Detour Scr_GetFunction_Detour;

BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
{
    if (pName)
    {
        const BuiltinFunctionDef *function = gsc::Find(*pName, functions);
        if (function)
        {
            *type = function->type;
            return function->actionFunc;
        }
    }

    const BuiltinFunction pFunction = Scr_GetFunction_Detour.GetOriginal<decltype(Scr_GetFunction)>()(pName, type);
    DbgPrint("QOS MP: Scr_GetFunction called for %s, type %d pFunction %p\n", *pName, *type, pFunction);
    return pFunction;
}

Detour Scr_GetMethod_Detour;

BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
{
    if (pName)
    {
        const BuiltinMethodDef *method = gsc::Find(*pName, methods);
        if (method)
        {
            *type = method->type;
            return method->actionFunc;
        }
    }

    const BuiltinMethod pMethod = Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
    DbgPrint("QOS MP: Scr_GetMethod called for %s, type %d pMethod %p\n", *pName, *type, pMethod);
    return pMethod;
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
} // namespace qos
