#include "pch.h"
#include "stats.h"

namespace t4
{
namespace mp
{
namespace
{
cmd_function_s Cmd_UnlockStats_VAR;

const char *TableLookup(const StringTable *table, int row, int column)
{
    if (!table || row < 0 || column < 0 || row >= table->rowCount || column >= table->columnCount || !table->values)
    {
        return "";
    }

    const char *value = table->values[row * table->columnCount + column];
    return value ? value : "";
}

bool TryParseInt(const char *text, int *out)
{
    if (!text || !*text)
    {
        return false;
    }

    const char *p = text;
    if (*p == '-')
    {
        ++p;
    }

    if (*p < '0' || *p > '9')
    {
        return false;
    }

    *out = atoi(text);
    return true;
}

bool HasText(const char *text)
{
    return text && *text;
}

std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
                   [](char c) { return static_cast<char>(tolower(static_cast<unsigned char>(c))); });
    return value;
}

const StringTable *FindStringTable(const char *name)
{
    StringTable *table = nullptr;
    if (StringTable_GetAsset(name, &table) && table)
    {
        return table;
    }

    const std::string lowerName = ToLower(name);
    if (StringTable_GetAsset(lowerName.c_str(), &table) && table)
    {
        return table;
    }

    return nullptr;
}

void ExecuteCommand(const char *command)
{
    char commandLine[1024] = {};
    const size_t commandLength = std::strlen(command);

    if (commandLength >= sizeof(commandLine) - 1)
    {
        Com_Printf(0, "unlockstats: command too long: %s\n", command);
        return;
    }

    std::memcpy(commandLine, command, commandLength);
    commandLine[commandLength] = '\n';

    Cbuf_ExecuteBuffer(0, 0, commandLine);
}

void StatSet(int stat, int value)
{
    ExecuteCommand(va(const_cast<char *>("statset %i %i"), stat, value));
}

void UnlockRank()
{
    const StringTable *rankTable = FindStringTable("mp/ranktable.csv");
    int maxRank = 64;
    int minXp = 148680;
    int maxXp = 153950;

    if (rankTable)
    {
        for (int row = 1; row < rankTable->rowCount; ++row)
        {
            if (std::strcmp(TableLookup(rankTable, row, 0), "maxrank") == 0)
            {
                TryParseInt(TableLookup(rankTable, row, 1), &maxRank);
                continue;
            }

            int rank = 0;
            int xp = 0;
            if (TryParseInt(TableLookup(rankTable, row, 0), &rank) &&
                TryParseInt(TableLookup(rankTable, row, 7), &xp) && xp >= maxXp)
            {
                maxRank = rank;
                TryParseInt(TableLookup(rankTable, row, 2), &minXp);
                maxXp = xp;
            }
        }
    }

    StatSet(2301, maxXp);
    StatSet(2326, 10);
    StatSet(2350, maxRank);
    StatSet(2351, minXp);
    StatSet(2352, maxXp);
    StatSet(2353, maxXp);
    StatSet(251, maxRank);
    StatSet(252, maxRank);
}

void FlushChallengeGroup(int stateStat, int progressStat, int maxProgress, int &challengeCount)
{
    if (stateStat <= 0 || progressStat <= 0)
    {
        return;
    }

    // GSC treats 255 as a completed challenge state; 1..n are active tiers.
    StatSet(stateStat, 255);
    StatSet(progressStat, maxProgress);
    ++challengeCount;
}

void UnlockChallengeTable(const StringTable *challengeTable, int &challengeCount)
{
    int stateStat = 0;
    int progressStat = 0;
    int maxProgress = 0;

    for (int row = 1; row < challengeTable->rowCount; ++row)
    {
        int newStateStat = 0;
        if (TryParseInt(TableLookup(challengeTable, row, 2), &newStateStat))
        {
            FlushChallengeGroup(stateStat, progressStat, maxProgress, challengeCount);

            stateStat = newStateStat;
            TryParseInt(TableLookup(challengeTable, row, 3), &progressStat);
            maxProgress = 0;
        }

        if (stateStat <= 0)
        {
            continue;
        }

        int target = 0;
        if (TryParseInt(TableLookup(challengeTable, row, 4), &target) && target > maxProgress)
        {
            maxProgress = target;
        }
    }

    FlushChallengeGroup(stateStat, progressStat, maxProgress, challengeCount);
}

void UnlockChallenges(int &challengeCount)
{
    const StringTable *challengeList = FindStringTable("mp/challengetable.csv");
    if (!challengeList)
    {
        Com_Printf(0, "unlockstats: mp/challengetable.csv not found\n");
        return;
    }

    for (int row = 1; row < challengeList->rowCount; ++row)
    {
        const char *challengeTableName = TableLookup(challengeList, row, 4);
        if (!HasText(challengeTableName))
        {
            continue;
        }

        const StringTable *challengeTable = FindStringTable(challengeTableName);
        if (!challengeTable)
        {
            Com_Printf(0, "unlockstats: %s not found\n", challengeTableName);
            continue;
        }

        UnlockChallengeTable(challengeTable, challengeCount);
    }
}

void Cmd_UnlockStats_f()
{
    int challengeCount = 0;

    ExecuteCommand("exec mp/unlock_init.cfg");
    UnlockRank();
    UnlockChallenges(challengeCount);
    ExecuteCommand("updategamerprofile");

    Com_Printf(0, "unlockstats: queued stats and %i challenge groups\n", challengeCount);
}

// TU7 keeps the 8 KiB profile payload at the start of each controller's
// 0x10B60-byte storage record. Only the audited profile fields are accessed.
struct OfflineStatData
{
    unsigned char playerStats[8192];
    unsigned char storage[0x4218 - 8192];
    bool firstTimeRunning;
    bool statsFetched;
    bool statWriteNeeded;
    unsigned char remaining[0x10B60 - 0x421B];
};
static_assert(sizeof(OfflineStatData) == 0x10B60, "");
static_assert(offsetof(OfflineStatData, statsFetched) == 0x4219, "");
static_assert(offsetof(OfflineStatData, statWriteNeeded) == 0x421A, "");

struct OfflineStatsFile
{
    unsigned int version;
    unsigned char playerStats[8192];
};
static_assert(sizeof(OfflineStatsFile) == 0x2004, "");

OfflineStatData *const controllerStats = reinterpret_cast<OfflineStatData *>(0x85348C18);
const unsigned int OFFLINE_STATS_VERSION = 1;
bool offlineStatsLoaded[4] = {};
XUID offlineStatsUsers[4] = {};
Detour LiveStorage_ReadStats_Detour;
Detour LiveStorage_UploadStats_Detour;

XUID GetStatsUser(unsigned int controllerIndex)
{
    XUID xuid = 0;
    if (XUserGetXUID(controllerIndex, &xuid) != ERROR_SUCCESS)
        return 0;
    return xuid;
}

bool IsSignedInToLive(unsigned int controllerIndex)
{
    return XUserGetSigninState(controllerIndex) == eXUserSigninState_SignedInToLive;
}

bool IsOfflineGame()
{
    if (Dvar_GetBool("onlinegame"))
        return false;

    if (Dvar_GetBool("systemlink") || Dvar_GetBool("splitscreen"))
        return true;

    // The frontend temporarily clears splitscreen to expose the full map list.
    for (int client = 0; client < 4; ++client)
    {
        if (Menu_IsMenuOpenAndVisible(client, "menu_gamesetup_systemlink") ||
            Menu_IsMenuOpenAndVisible(client, "menu_gamesetup_splitscreen"))
            return true;
    }

    return false;
}

std::string GetStatsPath(unsigned int controllerIndex)
{
    char filename[64] = {};
    const XUID xuid = GetStatsUser(controllerIndex);
    if (xuid)
        _snprintf_s(filename, ARRAYSIZE(filename), _TRUNCATE, "%016I64X.stat", xuid);
    else
        _snprintf_s(filename, ARRAYSIZE(filename), _TRUNCATE, "controller_%u.stat", controllerIndex);

    return filesystem::JoinPath("codxe:\\players", filename);
}

unsigned int StatsChecksum(const unsigned char *playerStats)
{
    return LiveStorage_StatsChecksum(playerStats + 4, 8188, 0);
}

bool SaveStats(unsigned int controllerIndex)
{
    if (controllerIndex >= 4 || !controllerStats[controllerIndex].statsFetched)
        return false;

    OfflineStatData &data = controllerStats[controllerIndex];
    const unsigned int checksum = StatsChecksum(data.playerStats);
    std::memcpy(data.playerStats, &checksum, sizeof(checksum));
    OfflineStatsFile file = {};
    file.version = OFFLINE_STATS_VERSION;
    std::memcpy(file.playerStats, data.playerStats, sizeof(file.playerStats));
    const std::string path = GetStatsPath(controllerIndex);
    if (!filesystem::WriteFileToDisk(path.c_str(), reinterpret_cast<const char *>(&file), sizeof(file)))
    {
        Com_Printf(0, "[codxe][T4][Stats] Failed to save controller %u to %s\n", controllerIndex, path.c_str());
        return false;
    }

    data.statWriteNeeded = false;
    data.firstTimeRunning = false;
    Com_Printf(0, "[codxe][T4][Stats] Saved controller %u to %s\n", controllerIndex, path.c_str());
    return true;
}

bool LoadStats(unsigned int controllerIndex)
{
    const std::string path = GetStatsPath(controllerIndex);
    const std::string contents = filesystem::ReadFileToString(path);
    if (contents.size() != sizeof(OfflineStatsFile))
        return false;

    OfflineStatsFile file = {};
    std::memcpy(&file, contents.data(), sizeof(file));
    unsigned int checksum = 0;
    std::memcpy(&checksum, file.playerStats, sizeof(checksum));
    if (file.version != OFFLINE_STATS_VERSION || checksum != StatsChecksum(file.playerStats))
    {
        Com_Printf(0, "[codxe][T4][Stats] Invalid offline stats in %s\n", path.c_str());
        return false;
    }

    OfflineStatData &data = controllerStats[controllerIndex];
    std::memcpy(data.playerStats, file.playerStats, sizeof(data.playerStats));
    data.statsFetched = true;
    data.statWriteNeeded = false;
    data.firstTimeRunning = false;
    Com_Printf(0, "[codxe][T4][Stats] Loaded controller %u from %s\n", controllerIndex, path.c_str());
    return true;
}

void LoadOrInitializeStats(unsigned int controllerIndex)
{
    if (controllerIndex >= 4 || !IsOfflineGame() || IsSignedInToLive(controllerIndex))
        return;

    if (!LoadStats(controllerIndex))
    {
        OfflineStatData &data = controllerStats[controllerIndex];
        data.statsFetched = true;
        data.firstTimeRunning = true;
        LiveStorage_ResetStats(controllerIndex, false);
        const int localClientNum = CL_LocalClientNumFromControllerIndex(controllerIndex);
        Cbuf_ExecuteBuffer(localClientNum, controllerIndex, "exec mp/unlock_init.cfg");
        SaveStats(controllerIndex);
    }
    offlineStatsLoaded[controllerIndex] = true;
    offlineStatsUsers[controllerIndex] = GetStatsUser(controllerIndex);
}

void RestoreNativeStats(unsigned int controllerIndex)
{
    if (controllerIndex >= 4 || !offlineStatsLoaded[controllerIndex])
        return;

    if (offlineStatsUsers[controllerIndex] == GetStatsUser(controllerIndex) &&
        controllerStats[controllerIndex].statWriteNeeded)
        SaveStats(controllerIndex);

    offlineStatsLoaded[controllerIndex] = false;
    // The native read clears statsFetched before fetching the real Live profile.
    LiveStorage_ReadStats_Detour.GetOriginal<LiveStorage_ReadStats_t>()(controllerIndex);
}

void LiveStorage_ReadStats_Hook(unsigned int controllerIndex)
{
    if (controllerIndex < 4 && IsOfflineGame() && !IsSignedInToLive(controllerIndex))
    {
        LoadOrInitializeStats(controllerIndex);
        return;
    }

    if (controllerIndex < 4 && offlineStatsLoaded[controllerIndex])
    {
        RestoreNativeStats(controllerIndex);
        return;
    }

    LiveStorage_ReadStats_Detour.GetOriginal<LiveStorage_ReadStats_t>()(controllerIndex);
}

DWORD LiveStorage_UploadStats_Hook(unsigned int controllerIndex)
{
    if (controllerIndex < 4 && offlineStatsLoaded[controllerIndex] &&
        (!IsOfflineGame() || IsSignedInToLive(controllerIndex)))
    {
        RestoreNativeStats(controllerIndex);
        return ERROR_SUCCESS;
    }

    if (controllerIndex < 4 && IsOfflineGame() && !IsSignedInToLive(controllerIndex))
    {
        if (offlineStatsLoaded[controllerIndex] && controllerStats[controllerIndex].statWriteNeeded)
            SaveStats(controllerIndex);
        return ERROR_SUCCESS;
    }

    return LiveStorage_UploadStats_Detour.GetOriginal<LiveStorage_UploadStats_t>()(controllerIndex);
}

void RegisterCommands()
{
    Cmd_AddCommandInternal("unlockstats", Cmd_UnlockStats_f, &Cmd_UnlockStats_VAR);
}
} // namespace

void stats::OnCmdInit()
{
    RegisterCommands();
}

void stats::LoadOfflineStats(int localClientNum)
{
    if (localClientNum < 0 || localClientNum >= 4 || !IsOfflineGame())
        return;

    const int controllerIndex = CL_ControllerIndexFromClientNum(localClientNum);
    if (controllerIndex < 0 || controllerIndex >= 4 || IsSignedInToLive(controllerIndex))
        return;

    if (!offlineStatsLoaded[controllerIndex] || !controllerStats[controllerIndex].statsFetched ||
        offlineStatsUsers[controllerIndex] != GetStatsUser(controllerIndex))
        LoadOrInitializeStats(controllerIndex);
}

void stats::RestoreLiveStats()
{
    for (unsigned int controller = 0; controller < 4; ++controller)
        RestoreNativeStats(controller);
}

stats::stats()
{
    LiveStorage_ReadStats_Detour = Detour(LiveStorage_ReadStats, LiveStorage_ReadStats_Hook);
    LiveStorage_ReadStats_Detour.Install();
    LiveStorage_UploadStats_Detour = Detour(LiveStorage_UploadStats, LiveStorage_UploadStats_Hook);
    LiveStorage_UploadStats_Detour.Install();
}

stats::~stats()
{
    LiveStorage_UploadStats_Detour.Remove();
    LiveStorage_ReadStats_Detour.Remove();
}

} // namespace mp
} // namespace t4
