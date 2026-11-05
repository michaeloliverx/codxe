#include "pch.h"
#include "sv_bots.h"

namespace iw2
{
namespace mp
{
namespace
{
struct BotMovementInfo
{
    int buttons;
    unsigned char weapon;
    bool hasWeapon;
    int weaponSpawnCount;
    char forwardmove;
    char rightmove;
    bool hasAngles;
    float angles[3];
};

BotMovementInfo botMovement[IW2_MAX_CLIENTS];
char pendingBotName[32];
Detour clientConnectDetour;
Detour serverCommandDetour;
Detour botUserMoveDetour;
Detour enterWorldDetour;
Detour calcPingsDetour;

int ClientCount()
{
    if (!svs->clients || !*sv_maxclients)
        return 0;
    const int count = (*sv_maxclients)->current.integer;
    return count < IW2_MAX_CLIENTS ? count : IW2_MAX_CLIENTS;
}

bool IsBot(int clientNum)
{
    // NA_BOT is zero in IW2; SV_UpdateBots uses this same test.
    return clientNum >= 0 && clientNum < ClientCount() && svs->clients[clientNum].state >= 2 &&
           svs->clients[clientNum].remoteAddressType == 0;
}

BotMovementInfo *GetBot(scr_entref_t entref)
{
    GetPlayerEntity(entref);
    if (!IsBot(entref.entnum))
    {
        Scr_Error("bot control methods require a test client");
        return nullptr;
    }
    return &botMovement[entref.entnum];
}

char ClampMove(int value)
{
    return static_cast<char>(value < -127 ? -127 : (value > 127 ? 127 : value));
}

int AngleToShort(float angle)
{
    return static_cast<int>(angle * (65536.0f / 360.0f)) & 65535;
}

const char *ConnectClient(int clientNum, unsigned int scriptId)
{
    if (pendingBotName[0] && clientNum >= 0 && clientNum < ClientCount())
    {
        client_t *client = &svs->clients[clientNum];
        // IW2 reads userinfo in ClientConnect before setting CS_CONNECTED or calling SV_UserinfoChanged.
        if (client->state == 0 && client->remoteAddressType == 0)
            Info_SetValueForKey(client->userinfo, "name", pendingBotName);
    }
    return clientConnectDetour.GetOriginal<ClientConnect_t>()(clientNum, scriptId);
}

bool IsValidWeapon(const playerState_s &ps, int weaponIndex)
{
    // Check the bitset bounds before entering the stock routine, which also validates weapon slots.
    return weaponIndex > 0 && weaponIndex < 128 && (ps.weapons[weaponIndex >> 5] & (1u << (weaponIndex & 31))) != 0 &&
           BG_IsWeaponValid(&ps, weaponIndex) != 0;
}

unsigned char CommandWeapon(int clientNum, const playerState_s &ps)
{
    BotMovementInfo &bot = botMovement[clientNum];
    if (bot.hasWeapon && bot.weaponSpawnCount != ps.spawnCount)
        bot.hasWeapon = false;
    if (bot.hasWeapon && bot.weapon && !IsValidWeapon(ps, bot.weapon))
        bot.hasWeapon = false;
    if (!bot.hasWeapon && IsValidWeapon(ps, ps.weapon))
    {
        bot.weapon = static_cast<unsigned char>(ps.weapon);
        bot.hasWeapon = true;
        bot.weaponSpawnCount = ps.spawnCount;
    }
    // Keep the intended weapon while ladders/mantles temporarily clear ps.weapon, like a human's command.
    return bot.hasWeapon ? bot.weapon : static_cast<unsigned char>(ps.weapon);
}

void SelectWeapon(int clientNum, int weaponIndex)
{
    if (IsBot(clientNum) && g_entities[clientNum].client)
    {
        BotMovementInfo &bot = botMovement[clientNum];
        const playerState_s &ps = g_entities[clientNum].client->ps;
        if (weaponIndex >= 0 && weaponIndex < 128)
        {
            bot.weapon = static_cast<unsigned char>(weaponIndex);
            bot.hasWeapon = true;
            bot.weaponSpawnCount = ps.spawnCount;
        }
    }
}

void GameSendServerCommand(int clientNum, int type, const char *text)
{
    // The retail script weapon methods inline G_SelectWeaponIndex and send this command directly.
    if (text && text[0] == 'a' && text[1] == ' ' && IsBot(clientNum))
    {
        char *end;
        const long index = strtol(text + 2, &end, 10);
        if (end != text + 2 && !*end && index >= 0 && index < 128)
            SelectWeapon(clientNum, static_cast<int>(index));
    }
    serverCommandDetour.GetOriginal<SV_GameSendServerCommand_t>()(clientNum, type, text);
}

void EnterWorld(client_t *client, const usercmd_s *cmd)
{
    const int clientNum = client - svs->clients;
    if (clientNum >= 0 && clientNum < ClientCount())
        ZeroMemory(&botMovement[clientNum], sizeof(botMovement[clientNum]));
    enterWorldDetour.GetOriginal<SV_ClientEnterWorld_t>()(client, cmd);
}

void BotUserMove(client_t *client)
{
    if (!client || !client->gentity || !client->gentity->client)
        return;
    const int clientNum = client - svs->clients;
    if (!IsBot(clientNum))
        return;

    const gclient_s *gameClient = client->gentity->client;
    const BotMovementInfo &bot = botMovement[clientNum];
    usercmd_s cmd = {};
    cmd.serverTime = svs->time;
    // Killcam playback must not replace the live bot's selection with archived state.
    cmd.weapon = gameClient->archiveTime
                     ? (bot.hasWeapon ? bot.weapon : static_cast<unsigned char>(gameClient->ps.weapon))
                     : CommandWeapon(clientNum, gameClient->ps);
    cmd.offHandIndex = static_cast<unsigned char>(gameClient->ps.offHandIndex);

    // Keep the stock killcam guard: archived clients must not act on the live player.
    if (!gameClient->archiveTime)
    {
        cmd.buttons = bot.buttons;
        cmd.forwardmove = bot.forwardmove;
        cmd.rightmove = bot.rightmove;
    }
    for (int i = 0; i < 3; ++i)
    {
        const float angle = bot.hasAngles ? bot.angles[i] : gameClient->ps.viewangles[i];
        cmd.angles[i] = (AngleToShort(angle) - gameClient->ps.delta_angles[i]) & 65535;
    }
    client->deltaMessage = client->outgoingSequence - 1;
    SV_ClientThink(client, &cmd);
}

void CalcPings()
{
    calcPingsDetour.GetOriginal<SV_CalcPings_t>()();
    for (int i = 0; i < ClientCount(); ++i)
    {
        if (IsBot(i))
            svs->clients[i].ping = 0;
    }
}

struct BotAction
{
    const char *name;
    int buttons;
};

const BotAction actions[] = {
    {"fire", CMD_BUTTON_ATTACK},
    {"attack", CMD_BUTTON_ATTACK},
    {"melee", CMD_BUTTON_MELEE},
    {"activate", CMD_BUTTON_USE},
    {"use", CMD_BUTTON_USE},
    {"usereload", CMD_BUTTON_USE_RELOAD},
    {"reload", CMD_BUTTON_RELOAD},
    {"leanleft", CMD_BUTTON_LEAN_LEFT},
    {"leanright", CMD_BUTTON_LEAN_RIGHT},
    {"gostand", CMD_BUTTON_JUMP},
    {"gocrouch", CMD_BUTTON_CROUCH},
    {"goprone", CMD_BUTTON_PRONE},
    {"ads", CMD_BUTTON_ADS},
    {"holdbreath", CMD_BUTTON_HOLDBREATH},
    {"frag", CMD_BUTTON_FRAG},
    {"smoke", CMD_BUTTON_SMOKE},
};
} // namespace

void SVBots::OnVMShutdown()
{
    ZeroMemory(botMovement, sizeof(botMovement));
    pendingBotName[0] = '\0';
}

void GScr_AddTestClient()
{
    pendingBotName[0] = '\0';
    const unsigned int parameterCount = Scr_GetNumParam();
    if (parameterCount > 1)
        Scr_Error("Usage: addtestclient(<name>);");
    if (parameterCount == 1)
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
        memcpy(pendingBotName, name, j + 1);
    }
    gentity_s *ent = SV_AddTestClient();
    pendingBotName[0] = '\0';
    if (ent)
        Scr_AddEntityNum(static_cast<unsigned int>(ent - g_entities), 0);
}

void PlayerCmd_BotAction(scr_entref_t entref)
{
    BotMovementInfo *bot = GetBot(entref);
    if (!bot)
        return;
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: <bot> botaction(<+action or -action>);");
    const char *action = Scr_GetString(0);
    if (action[0] != '+' && action[0] != '-')
        Scr_Error("botaction requires a '+' or '-' prefix");
    for (size_t i = 0; i < ARRAYSIZE(actions); ++i)
    {
        if (!_stricmp(action + 1, actions[i].name))
        {
            if (action[0] == '+')
                bot->buttons |= actions[i].buttons;
            else
                bot->buttons &= ~actions[i].buttons;
            return;
        }
    }
    Scr_Error("unknown bot action");
}

void PlayerCmd_BotStop(scr_entref_t entref)
{
    BotMovementInfo *bot = GetBot(entref);
    if (!bot)
        return;
    if (Scr_GetNumParam() != 0)
        Scr_Error("Usage: <bot> botstop();");
    bot->buttons = 0;
    bot->forwardmove = 0;
    bot->rightmove = 0;
    bot->hasAngles = false;
    // Spawn/reset scripts call botstop after weapon selection; keep a valid pending selection.
    CommandWeapon(entref.entnum, g_entities[entref.entnum].client->ps);
}

void PlayerCmd_BotMovement(scr_entref_t entref)
{
    BotMovementInfo *bot = GetBot(entref);
    if (!bot)
        return;
    if (Scr_GetNumParam() != 2)
        Scr_Error("Usage: <bot> botmovement(<forward>, <right>);");
    const int forward = Scr_GetInt(0);
    const int right = Scr_GetInt(1);
    bot->forwardmove = ClampMove(forward);
    bot->rightmove = ClampMove(right);
}

void PlayerCmd_BotAngles(scr_entref_t entref)
{
    BotMovementInfo *bot = GetBot(entref);
    if (!bot)
        return;
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: <bot> botangles(<angles>);");
    Scr_GetVector(0, bot->angles);
    bot->hasAngles = true;
}

void PlayerCmd_IsBot(scr_entref_t entref)
{
    GetPlayerEntity(entref);
    Scr_AddBool(IsBot(entref.entnum));
}

void PlayerCmd_IsHost(scr_entref_t entref)
{
    GetPlayerEntity(entref);
    Scr_AddBool(entref.entnum < ClientCount() && svs->clients[entref.entnum].remoteAddressType == 2);
}

SVBots::SVBots()
{
    clientConnectDetour = Detour(ClientConnect, ConnectClient);
    clientConnectDetour.Install();
    serverCommandDetour = Detour(SV_GameSendServerCommand, GameSendServerCommand);
    serverCommandDetour.Install();
    botUserMoveDetour = Detour(SV_BotUserMove, BotUserMove);
    botUserMoveDetour.Install();
    enterWorldDetour = Detour(SV_ClientEnterWorld, EnterWorld);
    enterWorldDetour.Install();
    calcPingsDetour = Detour(SV_CalcPings, CalcPings);
    calcPingsDetour.Install();
}

SVBots::~SVBots()
{
    clientConnectDetour.Remove();
    serverCommandDetour.Remove();
    botUserMoveDetour.Remove();
    enterWorldDetour.Remove();
    calcPingsDetour.Remove();
    OnVMShutdown();
}
} // namespace mp
} // namespace iw2
