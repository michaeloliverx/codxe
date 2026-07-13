#include "pch.h"
#include "bots.h"

namespace iw5
{
namespace mp
{
dvar_t *sv_maxClients = nullptr;
static dvar_t *ResolveSvMaxClients()
{
    if (sv_maxClients == nullptr)
    {
        sv_maxClients = Dvar_FindMalleableVar("sv_maxclients");
    }
    return sv_maxClients;
}

static bool CanAddBot()
{
    auto count = 0;

    ResolveSvMaxClients();

    auto *clients = *svs_clients;

    if (sv_maxClients == nullptr)
    {
        DbgPrint("CanAddBot: sv_maxClients dvar is null\n");
        return false;
    }

    for (auto i = 0; i < sv_maxClients->current.integer; ++i)
    {
        client_t *client = &clients[i];

        if (client->header.state >= 1)
        {
            ++count;
        }
    }

    return count < sv_maxClients->current.integer;
}

static int GetAvailableClientSlot()
{
    auto *clients = *svs_clients;

    if (sv_maxClients == nullptr)
    {
        DbgPrint("GetAvailableClientSlot: sv_maxClients is null\n");
        return -1;
    }

    for (auto i = 1; i < sv_maxClients->current.integer; ++i)
    {
        client_t *client = &clients[i];
        auto state = client->header.state;
        if (state < 1)
        {
            return i;
        }
    }

    return -1;
}

static char *GetBotName(int slot)
{
    int id = slot >= 0 ? slot : *svs_numclients;
    return va("Bot_%d", id);
}

static uint32_t SpawnBotThread(SpawnBotOptions *opts)
{
    Sleep(150);

    Scr_AddString("autoassign");
    Scr_AddString("team_marinesopfor");
    Scr_NotifyNum(opts->entNum, 0, SL_GetString("menuresponse", 0), 2);

    Sleep(200);

    Scr_AddString("class0");
    Scr_AddString("changeclass");
    Scr_NotifyNum(opts->entNum, 0, SL_GetString("menuresponse", 0), 2);

    auto ent = &g_entities[opts->entNum];
    ent->client->sess.cs.rank = opts->level;
    ent->client->sess.cs.prestige = opts->prestige;

    delete opts;

    return 0;
}

static void AddBot()
{
    if (!CanAddBot())
    {
        DbgPrint("Cannot add bot, max clients reached\n");
        return;
    }

    auto firstAvailableSlot = GetAvailableClientSlot();

    auto botName = GetBotName(firstAvailableSlot);
    auto xuid1 = G_IRand(0, INT_MAX);
    auto xuid2 = G_IRand(0, INT_MAX);

    auto botLevel = G_IRand(0, 79);
    auto botPrestige = G_IRand(0, 20);

    auto connectString =
        va("connect bot%d "
           "\"\\rate\\20000\\name\\%s\\natType\\1\\rank\\%d\\prestige\\%d\\ec_usingTag\\0\\ec_usingTitle\\0\\ec_"
           "TitleBg\\0\\ec_"
           "Level\\0\\ptd_prestige_black_ops\\0\\ptd_rank_black_ops\\0\\ptd_prestige_mw2\\0\\ptd_rank_"
           "mw2\\0\\ptd_prestige_world_at_war\\0\\ptd_rank_world_at_war\\0\\ptd_prestige_mw\\0\\ptd_rank_"
           "mw\\0\\protocol\\%i\\checksum\\%i\\challenge\\0\\statver\\26 "
           "3648679816\\invited\\1\\xuid\\%08x%08x\\onlineStats\\0\\migrating\\0\"",
           firstAvailableSlot - 1, botName, botLevel, botPrestige, GetProtocolVersion(), BG_NetDataChecksum(), xuid1,
           xuid2);

    netadr_t botAddress = {};
    botAddress.type = NA_BOT;

    SV_Cmd_TokenizeString(connectString);
    SV_DirectConnect(botAddress, firstAvailableSlot - 1);
    SV_Cmd_EndTokenizedString();

    auto *clients = *svs_clients;

    if (!clients || clients[firstAvailableSlot].header.state < 1)
    {
        DbgPrint("AddBot: client slot %d not active after SV_DirectConnect, aborting\n", firstAvailableSlot);
        return;
    }

    client_t *pClient = &clients[firstAvailableSlot];
    pClient->scriptId = 1023;
    pClient->bIsTestClient = 1;

    usercmd_s cmd = {};
    SV_SendClientGameState(pClient);
    SV_ClientEnterWorld(pClient, &cmd);

    SpawnBotOptions *botOpts = new SpawnBotOptions();
    botOpts->entNum = pClient->gentity->s.number;
    botOpts->level = botLevel;
    botOpts->prestige = botPrestige;

    CreateThread(nullptr, 0, reinterpret_cast<PTHREAD_START_ROUTINE>(SpawnBotThread), botOpts, 0, nullptr);
}

void SpawnBot(scr_entref_t entref)
{
    if (!CanAddBot())
    {
        DbgPrint("Cannot add bot, max clients reached\n");
        return;
    }

    if (GetAvailableClientSlot() == -1)
    {
        DbgPrint("Cannot add bot, no available client slots\n");
        return;
    }

    auto count = 1;

    auto numParams = Scr_GetNumParam();

    if (numParams > 1)
    {
        count = Scr_GetInt(1);
    }

    auto numBots = min(count, (int)*svs_numclients);

    DbgPrint("Spawning %d bot(s)\n", numBots);

    for (auto i = 0; i < numBots; ++i)
    {
        // can this all happen too fast?
        AddBot();
    }
}

Detour SV_DropClient_Detour;
static void SV_DropClient_Hook(client_t *cl, const char *reason, bool tellThem)
{
    if (cl->bIsTestClient == 1)
        return;

    SV_DropClient_Detour.GetOriginal<SV_DropClient_t>()(cl, reason, tellThem);
}

Bots::Bots()
{
    SV_DropClient_Detour = Detour(SV_DropClient, SV_DropClient_Hook);
    SV_DropClient_Detour.Install();
}

Bots::~Bots()
{
    SV_DropClient_Detour.Remove();
}
} // namespace mp
} // namespace iw5