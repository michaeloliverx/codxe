#include "pch.h"
#include "events.h"
#include "g_scr_main.h"
#include "sv_bots.h"

namespace iw3
{
namespace mp
{

struct BotMovementInfo_t
{
    int buttons;
    unsigned char weapon;
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

Detour G_SelectWeaponIndex_Detour;
void G_SelectWeaponIndex_Hook(int clientNum, int iWeaponIndex)
{
    if (clientNum >= 0 && clientNum < MAX_CLIENTS)
        g_botai[clientNum].weapon = (unsigned char)iWeaponIndex;

    G_SelectWeaponIndex_Detour.GetOriginal<G_SelectWeaponIndex_t>()(clientNum, iWeaponIndex);
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

    cmd.weapon = g_botai[clientNum].weapon;

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

static char s_pendingBotName[32];

// Intercept SV_UserinfoChanged when called from SV_DirectConnect for a bot
// connecting (NA_BOT type, CS_CONNECTED state). At this point cl->userinfo
// has the raw connect string; we patch the "name" key before SV_UserinfoChanged
// reads it so the correct name propagates through SV_ClientEnterWorld and the
// subsequent SV_SetConfigstring broadcast to all clients.
Detour SV_UserinfoChanged_Detour;
void SV_UserinfoChanged_Hook(client_t *cl)
{
    if (s_pendingBotName[0] && cl->header.netchan.remoteAddress.type == NA_BOT && cl->header.state == CS_CONNECTED)
    {
        Info_SetValueForKey(cl->userinfo, "name", s_pendingBotName);
    }

    SV_UserinfoChanged_Detour.GetOriginal<SV_UserinfoChanged_t>()(cl);
}

static void GScr_AddTestClient()
{
    if (Scr_GetNumParam() == 1)
    {
        const char *string = Scr_GetString(0);

        // Strip non-printable chars, matching name field size (32 bytes = 31 chars + null)
        char name[32];
        int i, j;
        for (i = 0, j = 0; string[i] && j < (int)sizeof(name) - 1; ++i)
        {
            if ((unsigned char)string[i] >= 0x20)
            {
                name[j] = string[i];
                ++j;
            }
        }
        name[j] = '\0';

        if (j < 1)
            Scr_Error("AddTestClient(): name must be at least 1 character long");

        strncpy(s_pendingBotName, name, sizeof(s_pendingBotName) - 1);
        s_pendingBotName[sizeof(s_pendingBotName) - 1] = '\0';
    }

    gentity_s *ent = SV_AddTestClient();
    s_pendingBotName[0] = '\0';

    if (ent)
        Scr_AddEntityNum(ent->s.number, 0);
}

Detour SV_CalcPings_Detour;
void SV_CalcPings_Stub()
{
    static const dvar_s *sv_maxclients = Dvar_FindMalleableVar("sv_maxclients");

    for (int i = 0; i < sv_maxclients->current.integer; i++)
    {
        client_t *cl = &svs->clients[i];

        if (cl->header.state != CS_ACTIVE)
        {
            cl->ping = -1;
            continue;
        }
        if (!cl->gentity)
        {
            cl->ping = 1;
            continue;
        }
        if (cl->header.netchan.remoteAddress.type == NA_BOT)
        {
            cl->ping = 0;
            cl->lastPacketTime = svs->time;
            continue;
        }

        int count = 0;
        int total = 0;
        int delta;
        for (int j = 0; j < PACKET_BACKUP; j++)
        {
            if (cl->frames[j].messageAcked == 0xFFFFFFFF)
            {
                continue;
            }
            delta = cl->frames[j].messageAcked - cl->frames[j].messageSent;
            count++;
            total += delta;
        }
        for (int j = 0; j < PACKET_BACKUP; j++)
        {
            const clientSnapshot_t *frame = &cl->frames[j];
            if (frame->messageAcked > 0)
            {
                count++;
                total += frame->messageAcked - frame->messageSent;
            }
        }
        if (!count)
        {
            cl->ping = 999;
        }
        else
        {
            cl->ping = total / count;
            if (cl->ping > 999)
            {
                cl->ping = 999;
            }
        }
    }
}

sv_bots::sv_bots()
{
    G_SelectWeaponIndex_Detour = Detour(G_SelectWeaponIndex, G_SelectWeaponIndex_Hook);
    G_SelectWeaponIndex_Detour.Install();

    SV_BotUserMove_Detour = Detour(SV_BotUserMove, SV_BotUserMove_Stub);
    SV_BotUserMove_Detour.Install();

    SV_UserinfoChanged_Detour = Detour(SV_UserinfoChanged, SV_UserinfoChanged_Hook);
    SV_UserinfoChanged_Detour.Install();

    SV_CalcPings_Detour = Detour(SV_CalcPings, SV_CalcPings_Stub);
    SV_CalcPings_Detour.Install();

    Scr_AddFunction("addtestclient", GScr_AddTestClient, 0);

    Scr_AddMethod("botmoveto", Scr_BotMoveTo, 0);
    Scr_AddMethod("botaction", Scr_BotAction, 0);
    Scr_AddMethod("botmirror", Scr_BotMirror, 0);
    Scr_AddMethod("botstop", Scr_BotStop, 0);

    Events::OnVMShutdown(CleanBotArray);
}

sv_bots::~sv_bots()
{
    G_SelectWeaponIndex_Detour.Remove();
    SV_BotUserMove_Detour.Remove();
    SV_UserinfoChanged_Detour.Remove();
    SV_CalcPings_Detour.Remove();

    CleanBotArray();
    s_pendingBotName[0] = '\0';
}
} // namespace mp
} // namespace iw3
