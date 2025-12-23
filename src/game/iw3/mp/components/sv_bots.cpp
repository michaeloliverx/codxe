#include "pch.h"
#include "g_scr_main.h"
#include "sv_bots.h"

#define KEY_MASK_FIRE 1
#define KEY_MASK_SPRINT 2
#define KEY_MASK_MELEE 4
#define KEY_MASK_RELOAD 16
#define KEY_MASK_LEANLEFT 64
#define KEY_MASK_LEANRIGHT 128
#define KEY_MASK_PRONE 256
#define KEY_MASK_CROUCH 512
#define KEY_MASK_JUMP 1024
#define KEY_MASK_ADS_MODE 2048
#define KEY_MASK_TEMP_ACTION 4096
#define KEY_MASK_HOLDBREATH 8192
#define KEY_MASK_FRAG 16384
#define KEY_MASK_SMOKE 32768
#define KEY_MASK_NIGHTVISION 262144
#define KEY_MASK_ADS 524288
#define KEY_MASK_USE 8
#define KEY_MASK_USERELOAD 32
#define BUTTON_ATTACK KEY_MASK_FIRE
#define MAX_CLIENTS 18

namespace iw3
{
namespace mp
{

struct BotMovementInfo_t
{
    int buttons;
};

BotMovementInfo_t g_botai[MAX_CLIENTS];

Detour SV_BotUserMove_Detour;
void SV_BotUserMove_Stub(client_t *cl)
{
    if (!cl->gentity)
        return;

    const int clientNum = cl - svsHeader->clients;

    usercmd_s cmd;
    memset(&cmd, 0, sizeof(cmd));

    cmd.serverTime = svsHeader->time;

    const playerState_s *ps = SV_GameClientNum(clientNum);
    cmd.weapon = (unsigned char)ps->weapon;

    if (g_clients[clientNum].sess.archiveTime == 0)
    {
        cmd.buttons = g_botai[clientNum].buttons;
    }

    cl->header.deltaMessage = cl->header.netchan.outgoingSequence - 1;
    SV_ClientThink(cl, &cmd);
}

struct BotAction_t
{
    const char *action;
    int key;
};

const BotAction_t BotActions[] = {{"gostand", KEY_MASK_JUMP},          {"gocrouch", KEY_MASK_CROUCH},
                                  {"goprone", KEY_MASK_PRONE},         {"fire", KEY_MASK_FIRE},
                                  {"melee", KEY_MASK_MELEE},           {"frag", KEY_MASK_FRAG},
                                  {"smoke", KEY_MASK_SMOKE},           {"reload", KEY_MASK_RELOAD},
                                  {"sprint", KEY_MASK_SPRINT},         {"leanleft", KEY_MASK_LEANLEFT},
                                  {"leanright", KEY_MASK_LEANRIGHT},   {"ads", KEY_MASK_ADS_MODE | KEY_MASK_ADS},
                                  {"holdbreath", KEY_MASK_HOLDBREATH}, {"activate", KEY_MASK_USE}};

static void Scr_BotAction(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: <bot> botAction(<action>);");

    const char *action = Scr_GetString(0);

    if (action[0] != '+' && action[0] != '-')
        Scr_ParamError(0, "Sign for bot action must be '+' or '-'.");

    bool key_found = false;
    for (size_t i = 0; i < ARRAYSIZE(g_botai); ++i)
    {
        if (!stricmp(&action[1], BotActions[i].action))
        {
            key_found = true;
            if (action[0] == '+')
                g_botai[entref.entnum].buttons |= BotActions[i].key;
            else
                g_botai[entref.entnum].buttons &= ~(BotActions[i].key);

            return;
        }
    }

    if (!key_found)
    {
        Scr_ParamError(0, va("Unknown bot action."));
    }
}

static void Scr_BotStop(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 0)
        Scr_Error("Usage: <bot> botStop();");

    g_botai[entref.entnum].buttons = 0;
}

sv_bots::sv_bots()
{
    SV_BotUserMove_Detour = Detour(SV_BotUserMove, SV_BotUserMove_Stub);
    SV_BotUserMove_Detour.Install();

    Scr_AddMethod("botaction", Scr_BotAction, 0);
    Scr_AddMethod("botstop", Scr_BotStop, 0);
}

sv_bots::~sv_bots()
{
    SV_BotUserMove_Detour.Remove();
}
} // namespace mp
} // namespace iw3
