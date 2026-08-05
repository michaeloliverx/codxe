#include "pch.h"
#include "bots.h"

namespace iw5
{
namespace mp
{
namespace
{
const int IW5_MAX_CLIENTS = 18;
const unsigned short ENTITYNUM_NONE = 2047;

#define IW5_ANGLE2SHORT(x) ((int)((x) * 65536.0f / 360.0f) & 65535)

enum CmdButton
{
    CMD_BUTTON_ATTACK = 1 << 0,
    CMD_BUTTON_SPRINT = 1 << 1,
    CMD_BUTTON_MELEE = 1 << 2,
    CMD_BUTTON_ACTIVATE = 1 << 3,
    CMD_BUTTON_RELOAD = 1 << 4,
    CMD_BUTTON_USE_RELOAD = 1 << 5,
    CMD_BUTTON_LEAN_LEFT = 1 << 6,
    CMD_BUTTON_LEAN_RIGHT = 1 << 7,
    CMD_BUTTON_PRONE = 1 << 8,
    CMD_BUTTON_CROUCH = 1 << 9,
    CMD_BUTTON_UP = 1 << 10,
    CMD_BUTTON_ADS = 1 << 11,
    CMD_BUTTON_BREATH = 1 << 13,
    CMD_BUTTON_FRAG = 1 << 14,
    CMD_BUTTON_OFFHAND_SECONDARY = 1 << 15,
    CMD_BUTTON_THROW = 1 << 19,
    CMD_BUTTON_REMOTE = 1 << 20,
};

struct BotMovementInfo
{
    bool active;
    int buttons;
    Weapon weapon;
    bool hasMove;
    char forwardMove;
    char rightMove;
    bool hasAngles;
    float angles[3];
    bool hasRemoteAngles;
    char remoteAngles[2];
    unsigned short meleeChargeEnt;
    unsigned char meleeChargeDist;
};

struct BotAction
{
    const char *name;
    int button;
};

BotMovementInfo g_botai[IW5_MAX_CLIENTS];
dvar_t *sv_maxClients = nullptr;
unsigned int g_botPort = 0;

Detour G_SelectWeapon_Detour;

int *G_SelectWeapon_Hook(int clientNum, Weapon weapon)
{
    if (clientNum >= 0 && clientNum < IW5_MAX_CLIENTS)
        g_botai[clientNum].weapon = weapon;

    return G_SelectWeapon_Detour.GetOriginal<G_SelectWeapon_t>()(clientNum, weapon);
}

const BotAction BOT_ACTIONS[] = {
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

char ClampMove(int value)
{
    if (value < -127)
        return -127;
    if (value > 127)
        return 127;
    return static_cast<char>(value);
}

unsigned char ClampByte(int value)
{
    if (value < 0)
        return 0;
    if (value > 255)
        return 255;
    return static_cast<unsigned char>(value);
}

unsigned short ClampEntityNum(int value)
{
    if (value < 0 || value > ENTITYNUM_NONE)
        return ENTITYNUM_NONE;
    return static_cast<unsigned short>(value);
}

int GetMaxClients()
{
    if (!sv_maxClients)
        sv_maxClients = Dvar_FindMalleableVar("sv_maxclients");

    if (!sv_maxClients)
        return 0;

    const int maxClients = sv_maxClients->current.integer;
    return maxClients < IW5_MAX_CLIENTS ? maxClients : IW5_MAX_CLIENTS;
}

BotMovementInfo *GetBotInfo(scr_entref_t entref)
{
    if (entref.classnum != 0 || entref.entnum >= IW5_MAX_CLIENTS || !g_entities[entref.entnum].client)
    {
        Scr_ErrorInternal();
        return nullptr;
    }

    return &g_botai[entref.entnum];
}

gentity_s *AddTestClient()
{
    client_t *clients = *svs_clients;
    const int maxClients = GetMaxClients();
    DbgPrint("[codxe][IW5][Bots] AddTestClient begin: clients=%p maxClients=%d\n", clients, maxClients);
    if (!clients || maxClients <= 0)
    {
        DbgPrint("[codxe][IW5][Bots] AddTestClient abort: client array unavailable\n");
        return nullptr;
    }

    int clientNum = 0;
    for (; clientNum < maxClients; ++clientNum)
    {
        if (clients[clientNum].header.state == CON_DISCONNECTED)
            break;
    }

    if (clientNum == maxClients)
    {
        DbgPrint("[codxe][IW5][Bots] AddTestClient abort: no free client slot\n");
        return nullptr;
    }

    DbgPrint("[codxe][IW5][Bots] AddTestClient selected slot %d\n", clientNum);

    const unsigned int botPort = g_botPort++;
    const int xuidHigh = G_IRand(0, INT_MAX);
    const int xuidLow = G_IRand(0, INT_MAX);
    const char *connectString =
        va("connect bot%u "
           "\"snaps\\20\\rate\\5000\\name\\Bot_%d\\natType\\1\\protocol\\%i\\checksum\\%i\\challenge\\0\\statver\\26 "
           "3648679816\\invited\\1\\xuid\\%08x%08x\\onlineStats\\0\\migrating\\0\\qport\\%u\"",
           botPort, clientNum, GetProtocolVersion(), BG_NetDataChecksum(), xuidHigh, xuidLow, botPort);

    netadr_t botAddress = {};
    botAddress.type = NA_BOT;
    botAddress.port = static_cast<unsigned short>(botPort);
    // IW5 ignores the port when comparing non-IP addresses. Give each bot a
    // distinct localNetID so Party_FindFirstMemberAtAddr does not treat every
    // NA_BOT connection as the first bot reconnecting.
    botAddress.localNetID = static_cast<netsrc_t>(NS_INVALID_NETSRC + clientNum + 1);

    DbgPrint("[codxe][IW5][Bots] AddTestClient slot %d: tokenizing connect string (port=%u localNetID=%d)\n", clientNum,
             botPort, botAddress.localNetID);
    SV_Cmd_TokenizeString(connectString);
    DbgPrint("[codxe][IW5][Bots] AddTestClient slot %d: entering SV_DirectConnect\n", clientNum);
    SV_DirectConnect(botAddress);
    DbgPrint("[codxe][IW5][Bots] AddTestClient slot %d: SV_DirectConnect returned\n", clientNum);
    SV_Cmd_EndTokenizedString();

    client_t *client = &clients[clientNum];
    if (client->header.state == CON_DISCONNECTED || !client->gentity)
    {
        DbgPrint("[codxe][IW5][Bots] AddTestClient slot %d abort: state=%d gentity=%p\n", clientNum,
                 client->header.state, client->gentity);
        return nullptr;
    }

    DbgPrint("[codxe][IW5][Bots] AddTestClient slot %d connected: state=%d gentity=%p\n", clientNum,
             client->header.state, client->gentity);

    client->scriptId = 1023;
    client->bIsTestClient = 1;
    client->gentity->s.number = clientNum;

    usercmd_s cmd = {};
    DbgPrint("[codxe][IW5][Bots] AddTestClient slot %d: entering SV_SendClientGameState\n", clientNum);
    SV_SendClientGameState(client);
    DbgPrint("[codxe][IW5][Bots] AddTestClient slot %d: entering SV_ClientEnterWorld\n", clientNum);
    SV_ClientEnterWorld(client, &cmd);
    DbgPrint("[codxe][IW5][Bots] AddTestClient slot %d: SV_ClientEnterWorld returned state=%d\n", clientNum,
             client->header.state);

    ZeroMemory(&g_botai[clientNum], sizeof(g_botai[clientNum]));
    g_botai[clientNum].meleeChargeEnt = ENTITYNUM_NONE;
    DbgPrint("[codxe][IW5][Bots] AddTestClient complete: slot=%d entity=%d\n", clientNum, client->gentity->s.number);
    return client->gentity;
}

Detour SV_ClientThink_Detour;

void SV_ClientThink_Hook(client_t *client, usercmd_s *cmd)
{
    // IW5's normal SV_UpdateBots path builds its usercmd inline, so SV_BotUserMove is not a reliable control seam.
    client_t *clients = *svs_clients;
    if (!client || !cmd || !clients)
    {
        SV_ClientThink_Detour.GetOriginal<decltype(SV_ClientThink)>()(client, cmd);
        return;
    }

    const int clientNum = client - clients;
    if (clientNum < 0 || clientNum >= GetMaxClients() || client->header.netchan.remoteAddress.type != NA_BOT)
    {
        SV_ClientThink_Detour.GetOriginal<decltype(SV_ClientThink)>()(client, cmd);
        return;
    }

    // Test clients do not send network packets back to the server, so they
    // never acknowledge reliable server commands through the normal path.
    client->reliableAcknowledge = client->reliableSequence;
    client->lastPacketTime = cmd->serverTime;
    if (!g_botai[clientNum].active)
    {
        SV_ClientThink_Detour.GetOriginal<decltype(SV_ClientThink)>()(client, cmd);
        return;
    }

    BotMovementInfo &bot = g_botai[clientNum];
    const playerState_s &ps = level->clients[clientNum].ps;

    // The player's current weapon is temporarily cleared while mantling. A
    // real client keeps sending its selected weapon in usercmd, so preserve
    // the last non-empty selection for test clients as well.
    if (ps.weapCommon.weapon.data != 0)
        bot.weapon = ps.weapCommon.weapon;

    usercmd_s botCmd = {};
    botCmd.serverTime = cmd->serverTime;
    botCmd.buttons = bot.buttons;
    botCmd.weapon = bot.weapon.data != 0 ? bot.weapon : ps.weapCommon.weapon;
    botCmd.offHand = ps.weapCommon.offHand;
    botCmd.forwardmove = bot.hasMove ? bot.forwardMove : 0;
    botCmd.rightmove = bot.hasMove ? bot.rightMove : 0;
    botCmd.meleeChargeEnt = bot.meleeChargeEnt;
    botCmd.meleeChargeDist = bot.meleeChargeDist;

    if (bot.hasAngles)
    {
        for (int i = 0; i < 3; ++i)
            botCmd.angles[i] = IW5_ANGLE2SHORT(bot.angles[i] - ps.delta_angles[i]);
    }

    if (bot.hasRemoteAngles)
    {
        botCmd.remoteControlAngles[0] = bot.remoteAngles[0];
        botCmd.remoteControlAngles[1] = bot.remoteAngles[1];
    }

    SV_ClientThink_Detour.GetOriginal<decltype(SV_ClientThink)>()(client, &botCmd);
}
} // namespace

void ResetBotState()
{
    ZeroMemory(g_botai, sizeof(g_botai));
    for (int i = 0; i < IW5_MAX_CLIENTS; ++i)
        g_botai[i].meleeChargeEnt = ENTITYNUM_NONE;
}

void GScr_AddTestClient()
{
    gentity_s *entity = AddTestClient();
    if (entity)
        Scr_AddEntityNum(entity->s.number, 0);
}

void PlayerCmd_BotAction(scr_entref_t entref)
{
    // Parameter 0 is the selector consumed by the getviewmodel trampoline.
    BotMovementInfo *bot = GetBotInfo(entref);
    if (!bot || Scr_GetNumParam() != 2)
    {
        Scr_ErrorInternal();
        return;
    }

    const char *action = Scr_GetString(1);
    if (!action || (action[0] != '+' && action[0] != '-'))
    {
        Scr_ErrorInternal();
        return;
    }

    for (size_t i = 0; i < ARRAYSIZE(BOT_ACTIONS); ++i)
    {
        if (_stricmp(&action[1], BOT_ACTIONS[i].name) == 0)
        {
            if (action[0] == '+')
                bot->buttons |= BOT_ACTIONS[i].button;
            else
                bot->buttons &= ~BOT_ACTIONS[i].button;

            bot->active = true;
            return;
        }
    }

    Scr_ErrorInternal();
}

void PlayerCmd_BotStop(scr_entref_t entref)
{
    BotMovementInfo *bot = GetBotInfo(entref);
    if (!bot || Scr_GetNumParam() != 1)
    {
        Scr_ErrorInternal();
        return;
    }

    bot->buttons = 0;
    bot->active = true;
    bot->hasMove = false;
    bot->hasRemoteAngles = false;
    bot->meleeChargeEnt = ENTITYNUM_NONE;
    bot->meleeChargeDist = 0;

    const playerState_s &ps = level->clients[entref.entnum].ps;
    for (int i = 0; i < 3; ++i)
        bot->angles[i] = ps.viewangles[i];
}

void PlayerCmd_BotMovement(scr_entref_t entref)
{
    BotMovementInfo *bot = GetBotInfo(entref);
    if (!bot || Scr_GetNumParam() != 3)
    {
        Scr_ErrorInternal();
        return;
    }

    const int forwardMove = Scr_GetInt(1);
    const int rightMove = Scr_GetInt(2);

    bot->forwardMove = ClampMove(forwardMove);
    bot->rightMove = ClampMove(rightMove);
    bot->active = true;
    bot->hasMove = true;
}

void PlayerCmd_BotMeleeParams(scr_entref_t entref)
{
    BotMovementInfo *bot = GetBotInfo(entref);
    if (!bot || Scr_GetNumParam() != 3)
    {
        Scr_ErrorInternal();
        return;
    }

    bot->meleeChargeEnt = ClampEntityNum(Scr_GetInt(1));
    bot->meleeChargeDist = ClampByte(static_cast<int>(Scr_GetFloat(2)));
    bot->active = true;
}

void PlayerCmd_BotRemoteAngles(scr_entref_t entref)
{
    BotMovementInfo *bot = GetBotInfo(entref);
    if (!bot || Scr_GetNumParam() != 3)
    {
        Scr_ErrorInternal();
        return;
    }

    bot->remoteAngles[0] = ClampMove(static_cast<int>(Scr_GetFloat(1)));
    bot->remoteAngles[1] = ClampMove(static_cast<int>(Scr_GetFloat(2)));
    bot->active = true;
    bot->hasRemoteAngles = true;
}

void PlayerCmd_BotAngles(scr_entref_t entref)
{
    BotMovementInfo *bot = GetBotInfo(entref);
    if (!bot || Scr_GetNumParam() != 4)
    {
        Scr_ErrorInternal();
        return;
    }

    for (int i = 0; i < 3; ++i)
        bot->angles[i] = Scr_GetFloat(i + 1);

    bot->active = true;
    bot->hasAngles = true;
}

Bots::Bots()
{
    ResetBotState();
    G_SelectWeapon_Detour = Detour(G_SelectWeapon, G_SelectWeapon_Hook);
    G_SelectWeapon_Detour.Install();

    SV_ClientThink_Detour = Detour(SV_ClientThink, SV_ClientThink_Hook);
    SV_ClientThink_Detour.Install();
}

Bots::~Bots()
{
    G_SelectWeapon_Detour.Remove();
    SV_ClientThink_Detour.Remove();
    ResetBotState();
}
} // namespace mp
} // namespace iw5
