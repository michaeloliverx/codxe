#include "pch.h"
#include <cctype>
#include <stdlib.h>
#include "command.h"
#include "stats.h"

namespace iw3
{
namespace mp
{
namespace
{
struct OfflineStatsFile
{
    unsigned int version;
    unsigned __int8 playerStats[8192];
};
static_assert(sizeof(OfflineStatsFile) == 0x2004, "");

const unsigned int OFFLINE_STATS_VERSION = 1;

Detour LiveStorage_ReadStats_Detour;
Detour LiveStorage_UploadStats_Detour;

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

bool Equals(const char *lhs, const char *rhs)
{
    return I_stricmp(lhs, rhs) == 0;
}

bool StartsWith(const char *text, const char *prefix)
{
    if (!text || !prefix)
    {
        return false;
    }

    while (*prefix)
    {
        if (*text != *prefix)
        {
            return false;
        }

        ++text;
        ++prefix;
    }

    return true;
}

std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
                   [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
    return value;
}

const StringTable *FindStringTable(const char *name)
{
    XAssetHeader header = DB_FindXAssetHeader(ASSET_TYPE_STRINGTABLE, name);
    if (header.stringTable)
    {
        return header.stringTable;
    }

    const std::string lowerName = ToLower(name);
    header = DB_FindXAssetHeader(ASSET_TYPE_STRINGTABLE, lowerName.c_str());
    return header.stringTable;
}

void ExecuteCommand(const char *command)
{
    char commandLine[1024] = {};
    const size_t commandLength = std::strlen(command);

    if (commandLength >= sizeof(commandLine) - 1)
    {
        Com_Printf(CON_CHANNEL_DONT_FILTER, "unlockstats: command too long: %s\n", command);
        return;
    }

    std::memcpy(commandLine, command, commandLength);
    commandLine[commandLength] = '\n';

    Cbuf_ExecuteBuffer(0, 0, commandLine);
}

void StatSet(unsigned int controllerIndex, int stat, int value)
{
    LiveStorage_SetStat(controllerIndex, stat, value);
}

void BuildUnlockBitMasks(std::map<std::string, int> &attachmentBits, int &allCamoBits)
{
    const StringTable *attachmentTable = FindStringTable("mp/attachmenttable.csv");
    if (!attachmentTable)
    {
        Com_Printf(CON_CHANNEL_DONT_FILTER, "unlockstats: mp/attachmenttable.csv not found\n");
        return;
    }

    allCamoBits = 0;

    for (int row = 1; row < attachmentTable->rowCount; ++row)
    {
        const char *name = TableLookup(attachmentTable, row, 4);
        int bit = 0;
        if (!HasText(name) || !TryParseInt(TableLookup(attachmentTable, row, 10), &bit) || !bit)
        {
            continue;
        }

        attachmentBits[name] = bit;

        if (Equals(TableLookup(attachmentTable, row, 2), "camo"))
        {
            allCamoBits |= bit;
        }
    }
}

int GetAttachmentMask(const std::map<std::string, int> &attachmentBits, const char *attachmentList)
{
    int mask = 0;
    const std::map<std::string, int>::const_iterator none = attachmentBits.find("none");
    if (none != attachmentBits.end())
    {
        mask |= none->second;
    }

    std::stringstream stream(attachmentList ? attachmentList : "");
    std::string token;
    while (stream >> token)
    {
        const std::map<std::string, int>::const_iterator bit = attachmentBits.find(token);
        if (bit != attachmentBits.end())
        {
            mask |= bit->second;
        }
    }

    return mask;
}

bool HasCamos(const char *category)
{
    return Equals(category, "weapon_smg") || Equals(category, "weapon_assault") || Equals(category, "weapon_sniper") ||
           Equals(category, "weapon_shotgun") || Equals(category, "weapon_lmg");
}

void UnlockItems(unsigned int controllerIndex, int &weaponCount, int &itemCount)
{
    const StringTable *statsTable = FindStringTable("mp/statstable.csv");
    if (!statsTable)
    {
        Com_Printf(CON_CHANNEL_DONT_FILTER, "unlockstats: mp/statstable.csv not found\n");
        return;
    }

    std::map<std::string, int> attachmentBits;
    int allCamoBits = 0;
    BuildUnlockBitMasks(attachmentBits, allCamoBits);

    for (int row = 1; row < statsTable->rowCount; ++row)
    {
        int stat = 0;
        if (!TryParseInt(TableLookup(statsTable, row, 1), &stat))
        {
            continue;
        }

        const char *category = TableLookup(statsTable, row, 2);
        const char *name = TableLookup(statsTable, row, 4);
        if (!HasText(category) || !HasText(name))
        {
            continue;
        }

        if (StartsWith(category, "weapon_"))
        {
            int mask = GetAttachmentMask(attachmentBits, TableLookup(statsTable, row, 8));
            if (HasCamos(category))
            {
                mask |= allCamoBits;
            }

            StatSet(controllerIndex, stat, mask);
            ++weaponCount;
        }
        else if (Equals(category, "specialty") || Equals(category, "grenade") || Equals(category, "specialgrenade") ||
                 Equals(category, "inventory") || Equals(category, "null_specialty") || Equals(category, "feature"))
        {
            StatSet(controllerIndex, stat, 1);
            ++itemCount;
        }
    }

    for (int stat = 256; stat <= 269; ++stat)
    {
        StatSet(controllerIndex, stat, 1);
    }

    for (int stat = 270; stat <= 289; ++stat)
    {
        StatSet(controllerIndex, stat, 1);
    }
}

void UnlockRank(unsigned int controllerIndex)
{
    const StringTable *rankTable = FindStringTable("mp/ranktable.csv");
    int maxRank = 54;
    int minXp = 120280;
    int maxXp = 125490;

    if (rankTable)
    {
        for (int row = 1; row < rankTable->rowCount; ++row)
        {
            if (Equals(TableLookup(rankTable, row, 0), "maxrank"))
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

    StatSet(controllerIndex, 2301, maxXp);
    StatSet(controllerIndex, 2326, 10);
    StatSet(controllerIndex, 2350, maxRank);
    StatSet(controllerIndex, 2351, minXp);
    StatSet(controllerIndex, 2352, maxXp);
    StatSet(controllerIndex, 2353, maxXp);
    StatSet(controllerIndex, 251, maxRank);
    StatSet(controllerIndex, 252, maxRank);
}

void FlushChallengeGroup(unsigned int controllerIndex, int stateStat, int progressStat, int maxProgress,
                         int &challengeCount)
{
    if (stateStat <= 0 || progressStat <= 0)
    {
        return;
    }

    // GSC treats 255 as a completed challenge state; 1..n are active tiers.
    StatSet(controllerIndex, stateStat, 255);
    StatSet(controllerIndex, progressStat, maxProgress);
    ++challengeCount;
}

void UnlockChallengeTable(unsigned int controllerIndex, const StringTable *challengeTable, int &challengeCount)
{
    int stateStat = 0;
    int progressStat = 0;
    int maxProgress = 0;

    for (int row = 1; row < challengeTable->rowCount; ++row)
    {
        int newStateStat = 0;
        if (TryParseInt(TableLookup(challengeTable, row, 2), &newStateStat))
        {
            FlushChallengeGroup(controllerIndex, stateStat, progressStat, maxProgress, challengeCount);

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

    FlushChallengeGroup(controllerIndex, stateStat, progressStat, maxProgress, challengeCount);
}

void UnlockChallenges(unsigned int controllerIndex, int &challengeCount)
{
    const StringTable *challengeList = FindStringTable("mp/challengetable.csv");
    if (!challengeList)
    {
        Com_Printf(CON_CHANNEL_DONT_FILTER, "unlockstats: mp/challengetable.csv not found\n");
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
            Com_Printf(CON_CHANNEL_DONT_FILTER, "unlockstats: %s not found\n", challengeTableName);
            continue;
        }

        UnlockChallengeTable(controllerIndex, challengeTable, challengeCount);
    }
}

void UnlockEverything(unsigned int controllerIndex)
{
    int weaponCount = 0;
    int itemCount = 0;
    int challengeCount = 0;

    UnlockRank(controllerIndex);
    UnlockItems(controllerIndex, weaponCount, itemCount);
    UnlockChallenges(controllerIndex, challengeCount);

    Com_Printf(CON_CHANNEL_DONT_FILTER,
               "[codxe][IW3][Stats] Unlocked controller %u: %i weapons, %i items, %i challenges\n", controllerIndex,
               weaponCount, itemCount, challengeCount);
}

void Cmd_UnlockStats_f()
{
    UnlockEverything(0);
    ExecuteCommand("updategamerprofile");
}

bool IsValidControllerIndex(unsigned int controllerIndex)
{
    return controllerIndex < 4;
}

std::string GetStatsPath(unsigned int controllerIndex)
{
    char filename[64] = {};
    XUID xuid = 0;

    if (XUserGetXUID(controllerIndex, &xuid) == ERROR_SUCCESS && xuid != 0)
    {
        _snprintf_s(filename, ARRAYSIZE(filename), _TRUNCATE, "%016I64X.stat", xuid);
    }
    else
    {
        _snprintf_s(filename, ARRAYSIZE(filename), _TRUNCATE, "controller_%u.stat", controllerIndex);
    }

    return filesystem::JoinPath("game:\\_codxe\\players", filename);
}

bool LoadStats(unsigned int controllerIndex)
{
    if (!IsValidControllerIndex(controllerIndex))
        return false;

    const std::string path = GetStatsPath(controllerIndex);
    const std::string contents = filesystem::ReadFileToString(path);
    if (contents.size() != sizeof(OfflineStatsFile))
        return false;

    const OfflineStatsFile *file = reinterpret_cast<const OfflineStatsFile *>(contents.data());
    if (file->version != OFFLINE_STATS_VERSION)
    {
        Com_PrintWarning(CON_CHANNEL_DONT_FILTER, "[codxe][IW3][Stats] Invalid offline header in %s\n", path.c_str());
        return false;
    }

    const unsigned int storedChecksum = *reinterpret_cast<const unsigned int *>(file->playerStats);
    const unsigned int calculatedChecksum = LiveStorage_ChecksumGamerStats(&file->playerStats[4]);
    if (storedChecksum != calculatedChecksum)
    {
        Com_PrintWarning(CON_CHANNEL_DONT_FILTER, "[codxe][IW3][Stats] Invalid offline checksum in %s\n", path.c_str());
        return false;
    }

    playerStatNetworkData &statData = controllerStatData[controllerIndex];
    std::memcpy(statData.playerStats, file->playerStats, sizeof(statData.playerStats));
    statData.statsFetched = true;
    statData.statWriteNeeded = false;
    statData.firstTimeRunning = false;

    Com_Printf(CON_CHANNEL_DONT_FILTER, "[codxe][IW3][Stats] Loaded controller %u from %s\n", controllerIndex,
               path.c_str());
    return true;
}

bool SaveStats(unsigned int controllerIndex)
{
    if (!IsValidControllerIndex(controllerIndex))
        return false;

    playerStatNetworkData &statData = controllerStatData[controllerIndex];
    if (!statData.statsFetched)
        return false;

    *reinterpret_cast<unsigned int *>(statData.playerStats) = LiveStorage_ChecksumGamerStats(&statData.playerStats[4]);

    OfflineStatsFile file = {};
    file.version = OFFLINE_STATS_VERSION;
    std::memcpy(file.playerStats, statData.playerStats, sizeof(file.playerStats));

    const std::string path = GetStatsPath(controllerIndex);
    if (!filesystem::WriteFileToDisk(path.c_str(), reinterpret_cast<const char *>(&file), sizeof(file)))
    {
        Com_PrintError(CON_CHANNEL_DONT_FILTER, "[codxe][IW3][Stats] Failed to save controller %u to %s\n",
                       controllerIndex, path.c_str());
        return false;
    }

    statData.statWriteNeeded = false;
    statData.firstTimeRunning = false;
    Com_Printf(CON_CHANNEL_DONT_FILTER, "[codxe][IW3][Stats] Saved controller %u to %s\n", controllerIndex,
               path.c_str());
    return true;
}

void InitializeStats(unsigned int controllerIndex)
{
    playerStatNetworkData &statData = controllerStatData[controllerIndex];
    statData.statsFetched = true;
    statData.firstTimeRunning = true;
    LiveStorage_ResetStats(controllerIndex);
    UnlockEverything(controllerIndex);
    SaveStats(controllerIndex);
}

void LoadOrInitializeStats(unsigned int controllerIndex)
{
    if (!IsValidControllerIndex(controllerIndex))
        return;

    if (!LoadStats(controllerIndex))
    {
        Com_Printf(CON_CHANNEL_DONT_FILTER,
                   "[codxe][IW3][Stats] No valid local stats for controller %u; creating defaults\n", controllerIndex);
        InitializeStats(controllerIndex);
    }
}

bool IsOfflineGame()
{
    return Dvar_GetBool("systemlink") || Dvar_GetBool("splitscreen");
}

void LiveStorage_ReadStats_Hook(unsigned int controllerIndex)
{
    if (XUserGetSigninState(controllerIndex) != eXUserSigninState_SignedInToLive)
    {
        LoadOrInitializeStats(controllerIndex);
        return;
    }

    LiveStorage_ReadStats_Detour.GetOriginal<LiveStorage_ReadStats_t>()(controllerIndex);
}

void LiveStorage_UploadStats_Hook(unsigned int controllerIndex)
{
    if (IsOfflineGame())
    {
        SaveStats(controllerIndex);
        return;
    }

    LiveStorage_UploadStats_Detour.GetOriginal<LiveStorage_UploadStats_t>()(controllerIndex);
}
} // namespace

void Stats::LoadStatsScript(int localClientNum, const char ** /*args*/)
{
    const int controllerIndex = CL_ControllerIndexFromClientNum(localClientNum);
    if (!IsValidControllerIndex(controllerIndex))
    {
        Com_PrintError(CON_CHANNEL_DONT_FILTER, "[codxe][IW3][Stats] Invalid controller %i for local client %i\n",
                       controllerIndex, localClientNum);
        return;
    }

    LoadOrInitializeStats(controllerIndex);
}

Stats::Stats()
{
    command::add("unlockstats", Cmd_UnlockStats_f);

    LiveStorage_ReadStats_Detour = Detour(LiveStorage_ReadStats, LiveStorage_ReadStats_Hook);
    LiveStorage_ReadStats_Detour.Install();

    LiveStorage_UploadStats_Detour = Detour(LiveStorage_UploadStats, LiveStorage_UploadStats_Hook);
    LiveStorage_UploadStats_Detour.Install();
}

Stats::~Stats()
{
    LiveStorage_ReadStats_Detour.Remove();
    LiveStorage_UploadStats_Detour.Remove();
}

} // namespace mp
} // namespace iw3
