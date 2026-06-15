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
        g_botai[clientNum].weapon = static_cast<unsigned short>(iWeaponIndex);

    return G_SelectWeaponIndex_Detour.GetOriginal<G_SelectWeaponIndex_t>()(clientNum, iWeaponIndex);
}

struct PerfBucket_t
{
    unsigned int totalMs;
    unsigned int maxMs;
    unsigned int calls;
};

static PerfBucket_t s_updateBotsPerf;
static PerfBucket_t s_gRunFramePerf;
static PerfBucket_t s_postFramePerf;
static PerfBucket_t s_runThinkPerf;
static PerfBucket_t s_xAnimPerf;
static PerfBucket_t s_clientNotifiesPerf;
static PerfBucket_t s_preThinkPerf;
static PerfBucket_t s_entityFramePerf;
static PerfBucket_t s_linkInfoPerf;
static PerfBucket_t s_clientEndFramePerf;
static PerfBucket_t s_scrThreadsPerf;
static unsigned int s_nextPerfPrint;

static void AddPerfSample(PerfBucket_t *bucket, unsigned int elapsed)
{
    bucket->totalMs += elapsed;
    if (elapsed > bucket->maxMs)
        bucket->maxMs = elapsed;

    ++bucket->calls;
}

static unsigned int GetPerfAverage(const PerfBucket_t *bucket)
{
    if (!bucket->calls)
        return 0;

    return bucket->totalMs / bucket->calls;
}

static void ResetPerfBucket(PerfBucket_t *bucket)
{
    bucket->totalMs = 0;
    bucket->maxMs = 0;
    bucket->calls = 0;
}

void MaybePrintPerf()
{
    const unsigned int now = Sys_Milliseconds();
    if (!s_nextPerfPrint)
        s_nextPerfPrint = now + 5000;

    if (now < s_nextPerfPrint)
        return;

    int bots = 0;
    for (int i = 0; i < svsHeader->clientCount && i < IW4_MAX_CLIENTS; ++i)
    {
        if (SV_IsClientBot(i) != FALSE)
            ++bots;
    }

    DbgPrint("[codxe][IW4 TU6][SVPerf] clients=%d bots=%d\n", svsHeader->clientCount, bots);
    DbgPrint("[codxe][IW4 TU6][SVPerf] updateBots total=%u avg=%u max=%u calls=%u\n", s_updateBotsPerf.totalMs,
             GetPerfAverage(&s_updateBotsPerf), s_updateBotsPerf.maxMs, s_updateBotsPerf.calls);
    DbgPrint("[codxe][IW4 TU6][SVPerf] gRunFrame total=%u avg=%u max=%u calls=%u\n", s_gRunFramePerf.totalMs,
             GetPerfAverage(&s_gRunFramePerf), s_gRunFramePerf.maxMs, s_gRunFramePerf.calls);
    DbgPrint("[codxe][IW4 TU6][SVPerf] postFrame total=%u avg=%u max=%u calls=%u\n", s_postFramePerf.totalMs,
             GetPerfAverage(&s_postFramePerf), s_postFramePerf.maxMs, s_postFramePerf.calls);
    DbgPrint("[codxe][IW4 TU6][SVPerf] runThink total=%u avg=%u max=%u calls=%u\n", s_runThinkPerf.totalMs,
             GetPerfAverage(&s_runThinkPerf), s_runThinkPerf.maxMs, s_runThinkPerf.calls);
    DbgPrint("[codxe][IW4 TU6][SVPerf] xAnim total=%u avg=%u max=%u calls=%u\n", s_xAnimPerf.totalMs,
             GetPerfAverage(&s_xAnimPerf), s_xAnimPerf.maxMs, s_xAnimPerf.calls);
    DbgPrint("[codxe][IW4 TU6][SVPerf] clientNotifies total=%u avg=%u max=%u calls=%u\n", s_clientNotifiesPerf.totalMs,
             GetPerfAverage(&s_clientNotifiesPerf), s_clientNotifiesPerf.maxMs, s_clientNotifiesPerf.calls);
    DbgPrint("[codxe][IW4 TU6][SVPerf] preThink total=%u avg=%u max=%u calls=%u\n", s_preThinkPerf.totalMs,
             GetPerfAverage(&s_preThinkPerf), s_preThinkPerf.maxMs, s_preThinkPerf.calls);
    DbgPrint("[codxe][IW4 TU6][SVPerf] entityFrame total=%u avg=%u max=%u calls=%u\n", s_entityFramePerf.totalMs,
             GetPerfAverage(&s_entityFramePerf), s_entityFramePerf.maxMs, s_entityFramePerf.calls);
    DbgPrint("[codxe][IW4 TU6][SVPerf] linkInfo total=%u avg=%u max=%u calls=%u\n", s_linkInfoPerf.totalMs,
             GetPerfAverage(&s_linkInfoPerf), s_linkInfoPerf.maxMs, s_linkInfoPerf.calls);
    DbgPrint("[codxe][IW4 TU6][SVPerf] clientEndFrame total=%u avg=%u max=%u calls=%u\n", s_clientEndFramePerf.totalMs,
             GetPerfAverage(&s_clientEndFramePerf), s_clientEndFramePerf.maxMs, s_clientEndFramePerf.calls);
    DbgPrint("[codxe][IW4 TU6][SVPerf] scrThreads total=%u avg=%u max=%u calls=%u\n", s_scrThreadsPerf.totalMs,
             GetPerfAverage(&s_scrThreadsPerf), s_scrThreadsPerf.maxMs, s_scrThreadsPerf.calls);

    ResetPerfBucket(&s_updateBotsPerf);
    ResetPerfBucket(&s_gRunFramePerf);
    ResetPerfBucket(&s_postFramePerf);
    ResetPerfBucket(&s_runThinkPerf);
    ResetPerfBucket(&s_xAnimPerf);
    ResetPerfBucket(&s_clientNotifiesPerf);
    ResetPerfBucket(&s_preThinkPerf);
    ResetPerfBucket(&s_entityFramePerf);
    ResetPerfBucket(&s_linkInfoPerf);
    ResetPerfBucket(&s_clientEndFramePerf);
    ResetPerfBucket(&s_scrThreadsPerf);
    s_nextPerfPrint = now + 5000;
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
        static_cast<unsigned short>(level->clients[clientNum].ps.weapCommon.primaryWeaponForAltMode);
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

static Detour SV_UpdateBots_Detour;

static void SV_UpdateBots_Hook()
{
    const unsigned int start = Sys_Milliseconds();
    SV_UpdateBots_Detour.GetOriginal<SV_UpdateBots_t>()();
    AddPerfSample(&s_updateBotsPerf, Sys_Milliseconds() - start);
}

static Detour G_RunFrame_Detour;

static void G_RunFrame_Hook(int levelTime)
{
    const unsigned int start = Sys_Milliseconds();
    G_RunFrame_Detour.GetOriginal<G_RunFrame_t>()(levelTime);
    AddPerfSample(&s_gRunFramePerf, Sys_Milliseconds() - start);
    // MaybePrintPerf();
}

static Detour SV_PostFrame_Detour;

static int *SV_PostFrame_Hook()
{
    const unsigned int start = Sys_Milliseconds();
    int *result = SV_PostFrame_Detour.GetOriginal<SV_PostFrame_t>()();
    AddPerfSample(&s_postFramePerf, Sys_Milliseconds() - start);
    return result;
}

static Detour G_RunThink_Detour;

static int G_RunThink_Hook(gentity_s *ent)
{
    const unsigned int start = Sys_Milliseconds();
    const int result = G_RunThink_Detour.GetOriginal<G_RunThink_t>()(ent);
    AddPerfSample(&s_runThinkPerf, Sys_Milliseconds() - start);
    return result;
}

static Detour G_XAnimUpdateEnt_Detour;

static void G_XAnimUpdateEnt_Hook(gentity_s *ent)
{
    const unsigned int start = Sys_Milliseconds();
    G_XAnimUpdateEnt_Detour.GetOriginal<G_XAnimUpdateEnt_t>()(ent);
    AddPerfSample(&s_xAnimPerf, Sys_Milliseconds() - start);
}

static Detour G_ClientDoPerFrameNotifies_Detour;

static void G_ClientDoPerFrameNotifies_Hook(gentity_s *ent)
{
    const unsigned int start = Sys_Milliseconds();
    G_ClientDoPerFrameNotifies_Detour.GetOriginal<G_ClientDoPerFrameNotifies_t>()(ent);
    AddPerfSample(&s_clientNotifiesPerf, Sys_Milliseconds() - start);
}

static Detour G_RunPreThinkForEntities_Detour;

static int *G_RunPreThinkForEntities_Hook()
{
    const unsigned int start = Sys_Milliseconds();
    int *result = G_RunPreThinkForEntities_Detour.GetOriginal<G_RunPreThinkForEntities_t>()();
    AddPerfSample(&s_preThinkPerf, Sys_Milliseconds() - start);
    return result;
}

static Detour G_RunFrameForEntity_Detour;

static void G_RunFrameForEntity_Hook(gentity_s *ent)
{
    const unsigned int start = Sys_Milliseconds();
    G_RunFrameForEntity_Detour.GetOriginal<G_RunFrameForEntity_t>()(ent);
    AddPerfSample(&s_entityFramePerf, Sys_Milliseconds() - start);
}

static Detour UpdateLinkInfoForClients_Detour;

static void UpdateLinkInfoForClients_Hook(gentity_s *ent)
{
    const unsigned int start = Sys_Milliseconds();
    UpdateLinkInfoForClients_Detour.GetOriginal<UpdateLinkInfoForClients_t>()(ent);
    AddPerfSample(&s_linkInfoPerf, Sys_Milliseconds() - start);
}

static Detour ClientEndFrame_Detour;

static void ClientEndFrame_Hook(gentity_s *ent)
{
    const unsigned int start = Sys_Milliseconds();
    ClientEndFrame_Detour.GetOriginal<ClientEndFrame_t>()(ent);
    AddPerfSample(&s_clientEndFramePerf, Sys_Milliseconds() - start);
}

static Detour Scr_RunCurrentThreads_Detour;

static void Scr_RunCurrentThreads_Hook()
{
    const unsigned int start = Sys_Milliseconds();
    Scr_RunCurrentThreads_Detour.GetOriginal<Scr_RunCurrentThreads_t>()();
    AddPerfSample(&s_scrThreadsPerf, Sys_Milliseconds() - start);
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
    const int dist = Scr_GetInt(1);

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

struct BotTrace_t
{
    float fraction;
    float normal[3];
    int surfaceFlags;
    int contents;
    const char *material;
    int hitType;
    unsigned short hitId;
    unsigned short modelIndex;
    unsigned short partName;
    unsigned short partGroup;
    bool allsolid;
    bool startsolid;
    bool walkable;
};

static_assert(offsetof(BotTrace_t, fraction) == 0x0, "");
static_assert(offsetof(BotTrace_t, surfaceFlags) == 0x10, "");
static_assert(offsetof(BotTrace_t, hitType) == 0x1C, "");
static_assert(sizeof(BotTrace_t) == 0x2C, "");

static bool BotCanSeePoint(float *eye, float *point)
{
    int sightHit = 0;
    G_SightTrace(&sightHit, eye, point, 2047, 8394755);
    if (sightHit != 0)
        return false;

    BotTrace_t trace;
    G_LocationalTrace(&trace, eye, point, 2047, 8415281, NULL);

    const int surfaceType = (trace.surfaceFlags >> 20) & 0x1F;
    return trace.fraction >= 1.0f || surfaceType == 9;
}

static void PlayerCmd_BotCanSeePlayer(scr_entref_t entref)
{
    GetPlayerEntity(entref);

    if (Scr_GetNumParam() != 6)
        Scr_Error("Usage: <bot> botCanSeePlayer(<player>, <eye>, <head>, <ankleLe>, <ankleRi>, <useAnkles>);");

    gentity_s *target = Scr_GetEntity(0);
    if (!target || !target->client)
        Scr_ParamError(0, "target is not a player");

    float eye[3];
    float head[3];
    float ankleLe[3];
    float ankleRi[3];
    Scr_GetVector(1, eye);
    Scr_GetVector(2, head);
    Scr_GetVector(3, ankleLe);
    Scr_GetVector(4, ankleRi);

    const bool useAnkles = Scr_GetInt(5) != 0;
    const bool canSee =
        BotCanSeePoint(eye, head) || (useAnkles && (BotCanSeePoint(eye, ankleLe) || BotCanSeePoint(eye, ankleRi)));
    Scr_AddInt(canSee ? 1 : 0);
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

    SV_UpdateBots_Detour = Detour(SV_UpdateBots, SV_UpdateBots_Hook);
    SV_UpdateBots_Detour.Install();

    G_RunFrame_Detour = Detour(G_RunFrame, G_RunFrame_Hook);
    G_RunFrame_Detour.Install();

    SV_PostFrame_Detour = Detour(SV_PostFrame, SV_PostFrame_Hook);
    SV_PostFrame_Detour.Install();

    G_RunThink_Detour = Detour(G_RunThink, G_RunThink_Hook);
    G_RunThink_Detour.Install();

    G_XAnimUpdateEnt_Detour = Detour(G_XAnimUpdateEnt, G_XAnimUpdateEnt_Hook);
    G_XAnimUpdateEnt_Detour.Install();

    G_ClientDoPerFrameNotifies_Detour = Detour(G_ClientDoPerFrameNotifies, G_ClientDoPerFrameNotifies_Hook);
    G_ClientDoPerFrameNotifies_Detour.Install();

    G_RunPreThinkForEntities_Detour = Detour(G_RunPreThinkForEntities, G_RunPreThinkForEntities_Hook);
    G_RunPreThinkForEntities_Detour.Install();

    G_RunFrameForEntity_Detour = Detour(G_RunFrameForEntity, G_RunFrameForEntity_Hook);
    G_RunFrameForEntity_Detour.Install();

    UpdateLinkInfoForClients_Detour = Detour(UpdateLinkInfoForClients, UpdateLinkInfoForClients_Hook);
    UpdateLinkInfoForClients_Detour.Install();

    ClientEndFrame_Detour = Detour(ClientEndFrame, ClientEndFrame_Hook);
    ClientEndFrame_Detour.Install();

    Scr_RunCurrentThreads_Detour = Detour(Scr_RunCurrentThreads, Scr_RunCurrentThreads_Hook);
    Scr_RunCurrentThreads_Detour.Install();

    Scr_AddFunction("addtestclient", GScr_AddTestClient, BUILTIN_ANY);

    Scr_AddMethod("botaction", PlayerCmd_BotAction, BUILTIN_ANY);
    Scr_AddMethod("botstop", PlayerCmd_BotStop, BUILTIN_ANY);
    Scr_AddMethod("botmovement", PlayerCmd_BotMovement, BUILTIN_ANY);
    Scr_AddMethod("botmeleeparams", PlayerCmd_BotMeleeParams, BUILTIN_ANY);
    Scr_AddMethod("botremoteangles", PlayerCmd_BotRemoteAngles, BUILTIN_ANY);
    Scr_AddMethod("botangles", PlayerCmd_BotAngles, BUILTIN_ANY);
    Scr_AddMethod("botcanseeplayer", PlayerCmd_BotCanSeePlayer, BUILTIN_ANY);
}

SVBots::~SVBots()
{
    G_SelectWeaponIndex_Detour.Remove();
    SV_BotUserMove_Detour.Remove();
    SV_CalcPings_Detour.Remove();
    SV_UpdateBots_Detour.Remove();
    G_RunFrame_Detour.Remove();
    SV_PostFrame_Detour.Remove();
    G_RunThink_Detour.Remove();
    G_XAnimUpdateEnt_Detour.Remove();
    G_ClientDoPerFrameNotifies_Detour.Remove();
    G_RunPreThinkForEntities_Detour.Remove();
    G_RunFrameForEntity_Detour.Remove();
    UpdateLinkInfoForClients_Detour.Remove();
    ClientEndFrame_Detour.Remove();
    Scr_RunCurrentThreads_Detour.Remove();

    CleanBotArray();
}

} // namespace mp_tu6
} // namespace iw4
