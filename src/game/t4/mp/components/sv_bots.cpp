#include "pch.h"
#include "events.h"
#include "sv_bots.h"

namespace t4
{
namespace mp
{

struct BotMovementInfo_t
{
    int buttons;
    unsigned char weapon;
    bool has_move;
    char forwardmove;
    char rightmove;
    bool has_angles;
    float angles[3];
    float melee_charge_yaw;
    unsigned char melee_charge_dist;
};

static BotMovementInfo_t g_botai[T4_MAX_CLIENTS];

static void CleanBotArray()
{
    ZeroMemory(&g_botai, sizeof(g_botai));
}

static gentity_s *GetPlayerEntity(scr_entref_t entref)
{
    if (entref.classnum != 0)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

    if (entref.entnum < 0 || entref.entnum >= T4_MAX_CLIENTS)
        Scr_ObjectError("entity index is out of range", SCRIPTINSTANCE_SERVER);

    gentity_s *ent = &g_entities[entref.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

    return ent;
}

static int ClampMove(int value)
{
    if (value < -127)
        return -127;
    if (value > 127)
        return 127;
    return value;
}

static unsigned char ClampByte(int value)
{
    if (value < 0)
        return 0;
    if (value > 255)
        return 255;
    return static_cast<unsigned char>(value);
}

Detour G_SelectWeaponIndex_Detour;

void G_SelectWeaponIndex_Hook(int clientNum, int iWeaponIndex)
{
    if (clientNum >= 0 && clientNum < T4_MAX_CLIENTS)
        g_botai[clientNum].weapon = static_cast<unsigned char>(iWeaponIndex);

    G_SelectWeaponIndex_Detour.GetOriginal<decltype(G_SelectWeaponIndex)>()(clientNum, iWeaponIndex);
}

Detour SV_BotUserMove_Detour;

void SV_BotUserMove_Stub(client_t *cl)
{
    if (!cl || !cl->gentity)
        return;

    const int clientNum = cl - svsHeader->clients;
    if (clientNum < 0 || clientNum >= T4_MAX_CLIENTS)
        return;

    usercmd_s cmd;
    memset(&cmd, 0, sizeof(cmd));

    cmd.serverTime = svsHeader->time;
    cmd.buttons = static_cast<button_mask>(g_botai[clientNum].buttons);
    cmd.weapon = g_botai[clientNum].weapon;
    cmd.forwardmove = g_botai[clientNum].has_move ? g_botai[clientNum].forwardmove : 0;
    cmd.rightmove = g_botai[clientNum].has_move ? g_botai[clientNum].rightmove : 0;
    cmd.meleeChargeYaw = g_botai[clientNum].melee_charge_yaw;
    cmd.meleeChargeDist = g_botai[clientNum].melee_charge_dist;

    if (g_botai[clientNum].has_angles)
    {
        const gclient_s *client = &level->clients[clientNum];
        cmd.angles[T4_PITCH] = ANGLE2SHORT(g_botai[clientNum].angles[T4_PITCH] - client->ps.delta_angles[T4_PITCH]);
        cmd.angles[T4_YAW] = ANGLE2SHORT(g_botai[clientNum].angles[T4_YAW] - client->ps.delta_angles[T4_YAW]);
        cmd.angles[T4_ROLL] = ANGLE2SHORT(g_botai[clientNum].angles[T4_ROLL] - client->ps.delta_angles[T4_ROLL]);
    }

    cl->header.deltaMessage = -1;
    SV_ClientThink(cl, &cmd);
}

struct BotAction_t
{
    const char *action;
    int key;
};

const BotAction_t BotActions[] = {
    {"gostand", KEY_GOSTAND},
    {"gocrouch", KEY_CROUCH},
    {"goprone", KEY_PRONE},
    {"fire", KEY_FIRE},
    {"attack", KEY_FIRE},
    {"melee", KEY_MELEE},
    {"frag", KEY_FRAG},
    {"smoke", KEY_SMOKE},
    {"reload", KEY_RELOAD},
    {"sprint", KEY_SPRINT},
    {"leanleft", KEY_LEANLEFT},
    {"leanright", KEY_LEANRIGHT},
    {"ads", KEY_ADSMODE | KEY_ADS},
    {"speed_throw", KEY_ADSMODE | KEY_ADS},
    {"holdbreath", KEY_HOLDBREATH},
    {"activate", KEY_USE},
    {"use", KEY_USE},
    {"crouch", KEY_CROUCH},
    {"prone", KEY_PRONE},
};

void PlayerCmd_BotAction(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("Usage: <bot> botAction(<action>);", SCRIPTINSTANCE_SERVER);

    const char *action = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if (action[0] != '+' && action[0] != '-')
        Scr_ParamError(0, "Sign for bot action must be '+' or '-'.", SCRIPTINSTANCE_SERVER);

    for (size_t i = 0; i < ARRAYSIZE(BotActions); ++i)
    {
        if (!_stricmp(&action[1], BotActions[i].action))
        {
            if (action[0] == '+')
                g_botai[entref.entnum].buttons |= BotActions[i].key;
            else
                g_botai[entref.entnum].buttons &= ~BotActions[i].key;

            return;
        }
    }

    char buffer[1024];
    buffer[0] = '\0';
    for (size_t i = 0; i < ARRAYSIZE(BotActions); ++i)
    {
        strncat(buffer, " ", sizeof(buffer) - strlen(buffer) - 1);
        strncat(buffer, BotActions[i].action, sizeof(buffer) - strlen(buffer) - 1);
    }

    Scr_ParamError(0, va("Unknown bot action. Must be one of:%s.", buffer), SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_BotStop(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 0)
        Scr_Error("Usage: <bot> botStop();", SCRIPTINSTANCE_SERVER);

    g_botai[entref.entnum].buttons = 0;
    g_botai[entref.entnum].has_move = false;
    g_botai[entref.entnum].melee_charge_yaw = 0.0f;
    g_botai[entref.entnum].melee_charge_dist = 0;
}

void PlayerCmd_BotMovement(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2)
        Scr_Error("Usage: <bot> botMovement(<forward>, <right>);", SCRIPTINSTANCE_SERVER);

    g_botai[entref.entnum].forwardmove = static_cast<char>(ClampMove(Scr_GetInt(0, SCRIPTINSTANCE_SERVER, 0, -1)));
    g_botai[entref.entnum].rightmove = static_cast<char>(ClampMove(Scr_GetInt(1, SCRIPTINSTANCE_SERVER, 0, -1)));
    g_botai[entref.entnum].has_move = true;
}

void PlayerCmd_BotMeleeParams(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2)
        Scr_Error("Usage: <bot> botMeleeParams(<yaw>, <dist>);", SCRIPTINSTANCE_SERVER);

    g_botai[entref.entnum].melee_charge_yaw = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    g_botai[entref.entnum].melee_charge_dist = ClampByte(Scr_GetInt(1, SCRIPTINSTANCE_SERVER, 0, -1));
}

void PlayerCmd_BotAngles(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("Usage: <bot> botAngles(<angles>);", SCRIPTINSTANCE_SERVER);

    float angles[3] = {0};
    Scr_GetVector(0, angles, SCRIPTINSTANCE_SERVER, -1);

    g_botai[entref.entnum].angles[T4_PITCH] = angles[T4_PITCH];
    g_botai[entref.entnum].angles[T4_YAW] = angles[T4_YAW];
    g_botai[entref.entnum].angles[T4_ROLL] = angles[T4_ROLL];
    g_botai[entref.entnum].has_angles = true;
}

void PlayerCmd_IsBot(scr_entref_t entref)
{
    GetPlayerEntity(entref);
    Scr_AddInt(SV_IsClientBot(entref.entnum), SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_IsHost(scr_entref_t entref)
{
    GetPlayerEntity(entref);
    Scr_AddInt(entref.entnum == 0 && !SV_IsClientBot(entref.entnum), SCRIPTINSTANCE_SERVER);
}

SVBots::SVBots()
{
    Events::OnVMShutdown(CleanBotArray);

    G_SelectWeaponIndex_Detour = Detour(G_SelectWeaponIndex, G_SelectWeaponIndex_Hook);
    G_SelectWeaponIndex_Detour.Install();

    SV_BotUserMove_Detour = Detour(SV_BotUserMove, SV_BotUserMove_Stub);
    SV_BotUserMove_Detour.Install();
}

SVBots::~SVBots()
{

    G_SelectWeaponIndex_Detour.Remove();

    SV_BotUserMove_Detour.Remove();

    CleanBotArray();
}

} // namespace mp
} // namespace t4
