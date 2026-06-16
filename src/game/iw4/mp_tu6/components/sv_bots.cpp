#include "pch.h"
#include "events.h"
#include "g_scr_main.h"
#include "sv_bots.h"

namespace iw4
{
namespace mp_tu6
{

struct BotMovementInfo_t
{
    bool active;
    int buttons;
    unsigned short weapon;
    unsigned short primaryWeaponForAlt;
    bool has_move;
    char forwardmove;
    char rightmove;
    bool has_angles;
    float angles[3];
    bool has_remote_angles;
    char remote_angles[2];
    float melee_charge_yaw;
    unsigned char melee_charge_dist;
};

static BotMovementInfo_t g_botai[IW4_MAX_CLIENTS];
static char s_pendingBotName[32];

static void CleanBotArray()
{
    ZeroMemory(&g_botai, sizeof(g_botai));
    s_pendingBotName[0] = '\0';
}

static char ClampMove(int value)
{
    if (value < -127)
        return -127;
    if (value > 127)
        return 127;
    return static_cast<char>(value);
}

static unsigned char ClampByte(int value)
{
    if (value < 0)
        return 0;
    if (value > 255)
        return 255;
    return static_cast<unsigned char>(value);
}

static Detour G_SelectWeaponIndex_Detour;

static int *G_SelectWeaponIndex_Hook(int clientNum, int iWeaponIndex)
{
    if (clientNum >= 0 && clientNum < IW4_MAX_CLIENTS)
    {
        g_botai[clientNum].weapon = static_cast<unsigned short>(iWeaponIndex);
        g_botai[clientNum].primaryWeaponForAlt = 0;

        const WeaponCompleteDef *def = BG_GetWeaponCompleteDef(iWeaponIndex);

        if (def && def->weapDef->inventoryType == WEAPINVENTORY_ALTMODE)
        {
            const playerState_s *ps = &g_entities[clientNum].client->ps;
            const int numWeaps = BG_GetNumWeapons();

            for (auto i = 1; i < numWeaps; i++)
            {
                if (!BG_PlayerHasWeapon(ps, i))
                    continue;

                const WeaponCompleteDef *thisDef = BG_GetWeaponCompleteDef(i);

                if (!thisDef || thisDef->altWeaponIndex != iWeaponIndex)
                    continue;

                g_botai[clientNum].primaryWeaponForAlt = static_cast<unsigned short>(i);
                break;
            }
        }
    }

    return G_SelectWeaponIndex_Detour.GetOriginal<G_SelectWeaponIndex_t>()(clientNum, iWeaponIndex);
}

static Detour SV_BotUserMove_Detour;

static void SV_BotUserMove_Stub(client_t *cl)
{
    if (!cl || !cl->gentity)
        return;

    const int clientNum = cl - svsHeader->clients;
    if (clientNum < 0 || clientNum >= svsHeader->clientCount || clientNum >= IW4_MAX_CLIENTS)
        return;

    const bool isBot = SV_IsClientBot(clientNum) != FALSE;
    const bool useStockFallback = !g_botai[clientNum].active && !isBot;

    if (useStockFallback)
    {
        SV_BotUserMove_Detour.GetOriginal<SV_BotUserMove_t>()(cl);
        return;
    }

    usercmd_s cmd;
    memset(&cmd, 0, sizeof(cmd));

    cmd.serverTime = svsHeader->time;
    cmd.buttons = g_botai[clientNum].buttons;
    cmd.weapon = g_botai[clientNum].weapon
                     ? g_botai[clientNum].weapon
                     : static_cast<unsigned short>(level->clients[clientNum].ps.weapCommon.weapon);
    cmd.primaryWeaponForAltMode =
        g_botai[clientNum].primaryWeaponForAlt
            ? g_botai[clientNum].primaryWeaponForAlt
            : static_cast<unsigned short>(level->clients[clientNum].ps.weapCommon.primaryWeaponForAltMode);
    cmd.offHandIndex = static_cast<unsigned short>(level->clients[clientNum].ps.weapCommon.offHandIndex);
    cmd.forwardmove = g_botai[clientNum].has_move ? g_botai[clientNum].forwardmove : 0;
    cmd.rightmove = g_botai[clientNum].has_move ? g_botai[clientNum].rightmove : 0;
    cmd.meleeChargeYaw = g_botai[clientNum].melee_charge_yaw;
    cmd.meleeChargeDist = g_botai[clientNum].melee_charge_dist;

    if (g_botai[clientNum].has_angles)
    {
        const gclient_s *client = &level->clients[clientNum];
        cmd.angles[IW4_PITCH] =
            IW4_ANGLE2SHORT(g_botai[clientNum].angles[IW4_PITCH] - client->ps.delta_angles[IW4_PITCH]);
        cmd.angles[IW4_YAW] = IW4_ANGLE2SHORT(g_botai[clientNum].angles[IW4_YAW] - client->ps.delta_angles[IW4_YAW]);
        cmd.angles[IW4_ROLL] = IW4_ANGLE2SHORT(g_botai[clientNum].angles[IW4_ROLL] - client->ps.delta_angles[IW4_ROLL]);
    }

    if (g_botai[clientNum].has_remote_angles)
    {
        cmd.remoteControlAngles[0] = g_botai[clientNum].remote_angles[0];
        cmd.remoteControlAngles[1] = g_botai[clientNum].remote_angles[1];
    }

    cl->header.deltaMessage = cl->header.netchan.outgoingSequence - 1;

    SV_ClientThink(cl, &cmd);
}

static Detour SV_CalcPings_Detour;

static void SV_CalcPings_Hook()
{
    SV_CalcPings_Detour.GetOriginal<SV_CalcPings_t>()();

    for (int i = 0; i < svsHeader->clientCount && i < IW4_MAX_CLIENTS; ++i)
    {
        if (SV_IsClientBot(i) != FALSE)
            svsHeader->clients[i].ping = 0;
    }
}

static void GScr_AddTestClient()
{
    if (Scr_GetNumParam() > 1)
        Scr_Error("Usage: addtestclient(<name>);");

    if (Scr_GetNumParam() == 1)
    {
        const char *string = Scr_GetString(0);

        char name[32];
        int i, j;
        for (i = 0, j = 0; string && string[i] && j < static_cast<int>(sizeof(name)) - 1; ++i)
        {
            if (static_cast<unsigned char>(string[i]) >= 0x20)
                name[j++] = string[i];
        }

        name[j] = '\0';

        if (j < 1)
            Scr_Error("addtestclient: name must be at least 1 character long");

        strncpy(s_pendingBotName, name, sizeof(s_pendingBotName) - 1);
        s_pendingBotName[sizeof(s_pendingBotName) - 1] = '\0';
    }

    gentity_s *ent = SV_AddTestClient();

    if (ent && s_pendingBotName[0])
    {
        const int clientNum = ent - g_entities;
        client_t *cl = &svsHeader->clients[clientNum];

        Info_SetValueForKey(cl->userinfo, "name", s_pendingBotName);
        SV_UserinfoChanged(cl);
    }

    s_pendingBotName[0] = '\0';

    if (ent)
        Scr_AddEntity(ent);
}

struct BotAction_t
{
    const char *action;
    int key;
};

static const BotAction_t BotActions[] = {
    {"gostand", CMD_BUTTON_UP},
    {"gocrouch", CMD_BUTTON_CROUCH},
    {"goprone", CMD_BUTTON_PRONE},
    {"fire", CMD_BUTTON_ATTACK},
    {"attack", CMD_BUTTON_ATTACK},
    {"melee", CMD_BUTTON_MELEE},
    {"frag", CMD_BUTTON_FRAG},
    {"smoke", CMD_BUTTON_OFFHAND_SECONDARY},
    {"reload", CMD_BUTTON_RELOAD},
    {"sprint", CMD_BUTTON_SPRINT},
    {"leanleft", CMD_BUTTON_LEAN_LEFT},
    {"leanright", CMD_BUTTON_LEAN_RIGHT},
    {"ads", CMD_BUTTON_ADS | CMD_BUTTON_THROW},
    {"speed_throw", CMD_BUTTON_ADS | CMD_BUTTON_THROW},
    {"holdbreath", CMD_BUTTON_BREATH},
    {"usereload", CMD_BUTTON_USE_RELOAD},
    {"activate", CMD_BUTTON_ACTIVATE},
    {"use", CMD_BUTTON_ACTIVATE},
    {"remote", CMD_BUTTON_REMOTE},
    {"crouch", CMD_BUTTON_CROUCH},
    {"prone", CMD_BUTTON_PRONE},
};

static void PlayerCmd_BotAction(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: <bot> botAction(<action>);");

    const char *action = Scr_GetString(0);
    if (action[0] != '+' && action[0] != '-')
        Scr_ParamError(0, "Sign for bot action must be '+' or '-'.");

    for (size_t i = 0; i < ARRAYSIZE(BotActions); ++i)
    {
        if (!_stricmp(&action[1], BotActions[i].action))
        {
            if (action[0] == '+')
                g_botai[entref.entnum].buttons |= BotActions[i].key;
            else
                g_botai[entref.entnum].buttons &= ~BotActions[i].key;

            g_botai[entref.entnum].active = true;
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

    Scr_ParamError(0, va("Unknown bot action. Must be one of:%s.", buffer));
}

static void PlayerCmd_BotStop(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 0)
        Scr_Error("Usage: <bot> botStop();");

    g_botai[entref.entnum].buttons = 0;
    g_botai[entref.entnum].active = true;
    g_botai[entref.entnum].has_move = false;
    g_botai[entref.entnum].has_remote_angles = false;
    g_botai[entref.entnum].melee_charge_yaw = 0.0f;
    g_botai[entref.entnum].melee_charge_dist = 0;
}

static void PlayerCmd_BotMovement(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 2)
        Scr_Error("Usage: <bot> botMovement(<forward>, <right>);");

    const int forward = Scr_GetInt(0);
    const int right = Scr_GetInt(1);

    g_botai[entref.entnum].forwardmove = ClampMove(forward);
    g_botai[entref.entnum].rightmove = ClampMove(right);
    g_botai[entref.entnum].active = true;
    g_botai[entref.entnum].has_move = true;
}

static void PlayerCmd_BotMeleeParams(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 2)
        Scr_Error("Usage: <bot> botMeleeParams(<yaw>, <dist>);");

    const float yaw = static_cast<float>(Scr_GetFloat(0));
    const int dist = static_cast<int>(static_cast<float>(Scr_GetFloat(1)));

    g_botai[entref.entnum].melee_charge_yaw = yaw;
    g_botai[entref.entnum].melee_charge_dist = ClampByte(dist);
    g_botai[entref.entnum].active = true;
}

static void PlayerCmd_BotRemoteAngles(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 2)
        Scr_Error("Usage: <bot> botRemoteAngles(<pitch>, <yaw>);");

    const int pitch = static_cast<int>(Scr_GetFloat(0));
    const int yaw = static_cast<int>(Scr_GetFloat(1));

    g_botai[entref.entnum].remote_angles[0] = ClampMove(pitch);
    g_botai[entref.entnum].remote_angles[1] = ClampMove(yaw);
    g_botai[entref.entnum].active = true;
    g_botai[entref.entnum].has_remote_angles = true;
}

static void PlayerCmd_BotAngles(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 3)
        Scr_Error("Usage: <bot> botAngles(<pitch>, <yaw>, <roll>);");

    const float pitch = static_cast<float>(Scr_GetFloat(0));
    const float yaw = static_cast<float>(Scr_GetFloat(1));
    const float roll = static_cast<float>(Scr_GetFloat(2));

    g_botai[entref.entnum].angles[IW4_PITCH] = pitch;
    g_botai[entref.entnum].angles[IW4_YAW] = yaw;
    g_botai[entref.entnum].angles[IW4_ROLL] = roll;
    g_botai[entref.entnum].active = true;
    g_botai[entref.entnum].has_angles = true;
}

SVBots::SVBots()
{
    Events::OnVMShutdown(CleanBotArray);

    G_SelectWeaponIndex_Detour = Detour(G_SelectWeaponIndex, G_SelectWeaponIndex_Hook);
    G_SelectWeaponIndex_Detour.Install();

    SV_BotUserMove_Detour = Detour(SV_BotUserMove, SV_BotUserMove_Stub);
    SV_BotUserMove_Detour.Install();

    SV_CalcPings_Detour = Detour(SV_CalcPings, SV_CalcPings_Hook);
    SV_CalcPings_Detour.Install();

    // remove bot check inside of Player_ActivateHoldCmd
    *(volatile uint32_t *)0x82263658 = 0x60000000;

    Scr_AddFunction("addtestclient", GScr_AddTestClient, BUILTIN_ANY);

    Scr_AddMethod("botaction", PlayerCmd_BotAction, BUILTIN_ANY);
    Scr_AddMethod("botstop", PlayerCmd_BotStop, BUILTIN_ANY);
    Scr_AddMethod("botmovement", PlayerCmd_BotMovement, BUILTIN_ANY);
    Scr_AddMethod("botmeleeparams", PlayerCmd_BotMeleeParams, BUILTIN_ANY);
    Scr_AddMethod("botremoteangles", PlayerCmd_BotRemoteAngles, BUILTIN_ANY);
    Scr_AddMethod("botangles", PlayerCmd_BotAngles, BUILTIN_ANY);
}

SVBots::~SVBots()
{
    G_SelectWeaponIndex_Detour.Remove();
    SV_BotUserMove_Detour.Remove();
    SV_CalcPings_Detour.Remove();

    CleanBotArray();
}

} // namespace mp_tu6
} // namespace iw4
