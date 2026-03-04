#include "pch.h"
#include "events.h"
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
    bool is_mirroring_client;
    int mirror_client_num;
    float moveTo[2];
    int doMove;
};

BotMovementInfo_t g_botai[MAX_CLIENTS];

static void CleanBotArray()
{
    ZeroMemory(&g_botai, sizeof(g_botai));
}

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

        if (g_botai[clientNum].doMove)
        {
            gentity_s *ent = cl->gentity;

            float move_pos[2];
            move_pos[0] = g_botai[clientNum].moveTo[0] - ent->r.currentOrigin[0];
            move_pos[1] = g_botai[clientNum].moveTo[1] - ent->r.currentOrigin[1];

            const float distance = sqrtf(move_pos[0] * move_pos[0] + move_pos[1] * move_pos[1]);
            g_botai[clientNum].doMove = distance > 7.0f ? 1 : 0;

            // Rotate offset into bot-local space by negative yaw
            const float yaw_rad = -ent->r.currentAngles[1] * (3.14159265358979323846f / 180.0f);
            const float s = sinf(yaw_rad);
            const float c = cosf(yaw_rad);
            const float rx = move_pos[0] * c - move_pos[1] * s;
            const float ry = move_pos[0] * s + move_pos[1] * c;
            move_pos[0] = rx;
            move_pos[1] = ry;

            // Scale to [-127, 127]
            const float absX = move_pos[0] < 0.0f ? -move_pos[0] : move_pos[0];
            const float absY = move_pos[1] < 0.0f ? -move_pos[1] : move_pos[1];
            const float maxabs = absX > absY ? absX : absY;
            if (maxabs > 0.0f)
            {
                move_pos[0] = move_pos[0] * (127.0f / maxabs);
                move_pos[1] = move_pos[1] * (127.0f / maxabs);
            }

            // Floor and invert Y to fit movement requirements
            move_pos[0] = floorf(move_pos[0]);
            move_pos[1] = -floorf(move_pos[1]);

            cmd.forwardmove = ((int)move_pos[0]) & 0xFF;
            cmd.rightmove = ((int)move_pos[1]) & 0xFF;

            if (!g_botai[clientNum].doMove)
            {
                static const auto scr_const_movedone = Scr_AllocString("movedone");
                Scr_Notify(ent, (unsigned __int16)scr_const_movedone, 0);
            }
        }

        // Handle mirrored mode
        // TODO: fix angles?
        if (g_botai[clientNum].is_mirroring_client)
        {
            const int mirror_num = g_botai[clientNum].mirror_client_num;
            if (mirror_num < MAX_CLIENTS)
            {
                const usercmd_s &lastUsercmd = svsHeader->clients[mirror_num].lastUsercmd;
                cmd.buttons = lastUsercmd.buttons;
                cmd.angles[PITCH] = lastUsercmd.angles[PITCH];
                cmd.angles[YAW] = lastUsercmd.angles[YAW];
                // Ignore ROLL
                cmd.forwardmove = lastUsercmd.forwardmove;
                cmd.rightmove = lastUsercmd.rightmove;
            }
        }
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

static void Scr_BotMoveTo(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: <bot> botMoveTo(<vec3 position>);");

    float moveTo[3];
    Scr_GetVector(0, moveTo);
    g_botai[entref.entnum].moveTo[0] = moveTo[0];
    g_botai[entref.entnum].moveTo[1] = moveTo[1];
    g_botai[entref.entnum].doMove = 1;
}

static void Scr_BotAction(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: <bot> botAction(<action>);");

    const char *action = Scr_GetString(0);

    if (action[0] != '+' && action[0] != '-')
        Scr_ParamError(0, "Sign for bot action must be '+' or '-'.");

    for (size_t i = 0; i < ARRAYSIZE(BotActions); ++i)
    {
        if (!stricmp(&action[1], BotActions[i].action))
        {
            if (action[0] == '+')
                g_botai[entref.entnum].buttons |= BotActions[i].key;
            else
                g_botai[entref.entnum].buttons &= ~(BotActions[i].key);

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

static void Scr_BotStop(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 0)
        Scr_Error("Usage: <bot> botStop();");

    g_botai[entref.entnum].buttons = 0;
    g_botai[entref.entnum].is_mirroring_client = false;
    g_botai[entref.entnum].doMove = 0;
}

static void Scr_BotMirror(scr_entref_t entref)
{
    // Validate self is a player entity
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: <bot> BotMirror(<client>);");

    const gentity_s *targetEntity = Scr_GetEntity(0);
    if (!targetEntity->client)
        Scr_Error("not a player");

    const int clientNum = targetEntity->client - g_clients;
    if (entref.entnum == clientNum)
    {
        Scr_Error("BotMirror: a bot cannot mirror itself.");
    }

    g_botai[entref.entnum].is_mirroring_client = true;
    g_botai[entref.entnum].mirror_client_num = clientNum;
}

sv_bots::sv_bots()
{
    SV_BotUserMove_Detour = Detour(SV_BotUserMove, SV_BotUserMove_Stub);
    SV_BotUserMove_Detour.Install();

    Scr_AddMethod("botmoveto", Scr_BotMoveTo, 0);
    Scr_AddMethod("botaction", Scr_BotAction, 0);
    Scr_AddMethod("botmirror", Scr_BotMirror, 0);
    Scr_AddMethod("botstop", Scr_BotStop, 0);

    Events::OnVMShutdown(CleanBotArray);
}

sv_bots::~sv_bots()
{
    SV_BotUserMove_Detour.Remove();
}
} // namespace mp
} // namespace iw3
