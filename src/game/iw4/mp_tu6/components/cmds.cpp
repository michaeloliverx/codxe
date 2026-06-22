#include "pch.h"
#include "cmds.h"

namespace iw4
{
namespace mp_tu6
{
namespace
{

Detour ClientCommand_Detour;

void SendCommandMessage(int clientNum, const char *message)
{
    const char *commandString = va("%c \"%s\"", 101, message);

    SV_GameSendServerCommand(clientNum, SV_CMD_CAN_IGNORE, commandString);
}

bool ToggleFlag(int &flags, int flag)
{
    const bool enableFlag = (flags & flag) == 0;

    if (enableFlag)
        flags |= flag;
    else
        flags &= ~flag;

    return enableFlag;
}

void Cmd_Noclip_f(int clientNum, gentity_s *ent)
{
    const bool enabled = ToggleFlag(ent->client->flags, CF_NOCLIP);
    SendCommandMessage(clientNum, enabled ? "GAME_NOCLIPON" : "GAME_NOCLIPOFF");
}

void Cmd_UFO_f(int clientNum, gentity_s *ent)
{
    const bool enabled = ToggleFlag(ent->client->flags, CF_UFO);
    SendCommandMessage(clientNum, enabled ? "GAME_UFOON" : "GAME_UFOOFF");
}

void Cmd_God_f(int clientNum, gentity_s *ent)
{
    const bool enabled = ToggleFlag(ent->flags, FL_GODMODE);
    SendCommandMessage(clientNum, enabled ? "GAME_GODMODE_ON" : "GAME_GODMODE_OFF");
}

void Cmd_DemiGod_f(int clientNum, gentity_s *ent)
{
    const bool enabled = ToggleFlag(ent->flags, FL_DEMI_GODMODE);
    SendCommandMessage(clientNum, enabled ? "GAME_DEMI_GODMODE_ON" : "GAME_DEMI_GODMODE_OFF");
}

void ClientCommand_Hook(int clientNum)
{
    assert(clientNum >= 0 && clientNum < IW4_MAX_CLIENTS);

    gentity_s *ent = &g_entities[clientNum];
    assert(ent->client);

    char cmd[1024] = {};
    SV_Cmd_ArgvBuffer(0, cmd, static_cast<int>(sizeof(cmd)));

    if (I_stricmp(cmd, "god") == 0)
        Cmd_God_f(clientNum, ent);
    else if (I_stricmp(cmd, "demigod") == 0)
        Cmd_DemiGod_f(clientNum, ent);
    else if (I_stricmp(cmd, "noclip") == 0)
        Cmd_Noclip_f(clientNum, ent);
    else if (I_stricmp(cmd, "ufo") == 0)
        Cmd_UFO_f(clientNum, ent);
    else
        ClientCommand_Detour.GetOriginal<decltype(ClientCommand)>()(clientNum);
}
} // namespace

cmds::cmds()
{
    ClientCommand_Detour = Detour(ClientCommand, ClientCommand_Hook);
    ClientCommand_Detour.Install();
}

cmds::~cmds()
{
    ClientCommand_Detour.Remove();
}
} // namespace mp_tu6
} // namespace iw4
