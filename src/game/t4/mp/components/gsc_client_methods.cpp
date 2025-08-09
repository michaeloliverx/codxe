#include "gsc_client_methods.h"
#include "common.h"

namespace t4
{
namespace mp
{
void PlayerCmd_SprintButtonPressed(scr_entref_t entref)
{
    if (entref.classnum != 0)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

    auto ent = &g_entities[entref.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> SprintButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_SPRINT) != 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_JumpButtonPressed(scr_entref_t entref)
{
    if (entref.classnum != 0)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

    auto ent = &g_entities[entref.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> JumpButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_GOSTAND) != 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_SetVelocity(scr_entref_t entref)
{
    float velocity[3] = {0};

    if (entref.classnum != 0)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

    auto ent = &g_entities[entref.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("Usage: <client> SetVelocity( vec3 )\n", SCRIPTINSTANCE_SERVER);

    Scr_GetVector(0, velocity, SCRIPTINSTANCE_SERVER, -1);

    ent->client->ps.velocity[0] = velocity[0];
    ent->client->ps.velocity[1] = velocity[1];
    ent->client->ps.velocity[2] = velocity[2];
}

void PlayerCmd_GetForwardMove(scr_entref_t arg)
{
    if (arg.classnum != 0)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

    auto ent = &g_entities[arg.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", arg.entnum), SCRIPTINSTANCE_SERVER);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 0)
        Scr_Error("Usage: <client> GetForwardMove()\n", SCRIPTINSTANCE_SERVER);

    auto cl = &svsHeader->clients[arg.entnum];

    Scr_AddInt(cl->lastUsercmd.forwardmove, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_GetRightMove(scr_entref_t arg)
{
    if (arg.classnum != 0)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

    auto ent = &g_entities[arg.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", arg.entnum), SCRIPTINSTANCE_SERVER);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 0)
        Scr_Error("Usage: <client> GetRightMove()\n", SCRIPTINSTANCE_SERVER);

    auto *cl = &svsHeader->clients[arg.entnum];

    Scr_AddInt(cl->lastUsercmd.rightmove, SCRIPTINSTANCE_SERVER);
}

void GScr_CloneBrushModelToScriptModel(scr_entref_t entref)
{
    static auto script_brushmodel = GScr_AllocString("script_brushmodel");
    static auto script_model = GScr_AllocString("script_model");
    static auto script_origin = GScr_AllocString("script_origin");
    static auto light = GScr_AllocString("light");

    // CoD4x
    // Common checks.
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("usage: <scriptModelEnt> CloneBrushModelToScriptModel(<brushModelEnt>)", SCRIPTINSTANCE_SERVER);

    auto scriptEnt = &g_entities[entref.entnum];
    if (scriptEnt->classname != script_model)
        Scr_ObjectError("passed entity is not a script_model entity", SCRIPTINSTANCE_SERVER);

    if (scriptEnt->s.eType != 6)
        Scr_ObjectError("passed entity type is not 6 (TODO: what is it?)", SCRIPTINSTANCE_SERVER);

    // Arguments checks.
    gentity_s *brushEnt = Scr_GetEntity(0);
    if (brushEnt->classname != script_brushmodel && brushEnt->classname != script_model &&
        brushEnt->classname != script_origin && brushEnt->classname != light)
        Scr_ParamError(
            0, "brush model entity classname must be one of {script_brushmodel, script_model, script_origin, light}",
            SCRIPTINSTANCE_SERVER);

    if (!brushEnt->s.index.brushmodel)
        Scr_ParamError(0, "brush model entity has no collision model", SCRIPTINSTANCE_SERVER);

    // Let's do this...
    SV_UnlinkEntity(scriptEnt);
    scriptEnt->s.index.brushmodel = brushEnt->s.index.brushmodel;
    int contents = scriptEnt->r.contents;
    SV_SetBrushModel(scriptEnt);
    scriptEnt->r.contents |= contents;
    SV_LinkEntity(scriptEnt);
}

static struct
{
    const char *name;
    BuiltinMethod handler;
} gsc_player_methods[] = {
    {"sprintbuttonpressed", PlayerCmd_SprintButtonPressed},
    {"jumpbuttonpressed", PlayerCmd_JumpButtonPressed},
    {"setvelocity", PlayerCmd_SetVelocity},
    {"getforwardmove", PlayerCmd_GetForwardMove},
    {"getrightmove", PlayerCmd_GetRightMove},
    {"clonebrushmodeltoscriptmodel", GScr_CloneBrushModelToScriptModel},
    {nullptr, nullptr} // Terminator
};

Detour Player_GetMethod_Detour;

BuiltinMethod Player_GetMethod_Hook(const char **pName)
{
    if (pName != nullptr)
    {
        const auto *func = gsc_player_methods;
        for (; func->name != nullptr; ++func)
        {
            if (_stricmp(*pName, func->name) == 0)
                return func->handler;
        }
    }

    return Player_GetMethod_Detour.GetOriginal<decltype(Player_GetMethod)>()(pName);
}

void DebugPrintBuiltinMethods()
{
    for (int i = 0; i < BUILTIN_PLAYER_METHOD_COUNT; ++i)
    {
        const auto &method = builtin_player_methods[i];
        if (method.actionFunc != nullptr)
        {
            DbgPrint("method[%d]: actionString: %s actionFunc: %p type: %d\n", i, method.actionString,
                     (void *)method.actionFunc, method.type);
        }
    }
}

GSCClientMethods::GSCClientMethods()
{
    DbgPrint("GSCClientMethods initialized\n");

    Player_GetMethod_Detour = Detour(Player_GetMethod, Player_GetMethod_Hook);
    Player_GetMethod_Detour.Install();
}

GSCClientMethods::~GSCClientMethods()
{
    DbgPrint("GSCClientMethods shutdown\n");
    Player_GetMethod_Detour.Remove();
}
} // namespace mp
} // namespace t4
