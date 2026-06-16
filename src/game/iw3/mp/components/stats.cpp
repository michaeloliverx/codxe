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

void StatSet(int stat, int value)
{
    ExecuteCommand(va(const_cast<char *>("statset %i %i"), stat, value));
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

void UnlockItems(int &weaponCount, int &itemCount)
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

            StatSet(stat, mask);
            ++weaponCount;
        }
        else if (Equals(category, "specialty") || Equals(category, "grenade") || Equals(category, "specialgrenade") ||
                 Equals(category, "inventory") || Equals(category, "null_specialty") || Equals(category, "feature"))
        {
            StatSet(stat, 1);
            ++itemCount;
        }
    }

    for (int stat = 256; stat <= 269; ++stat)
    {
        StatSet(stat, 1);
    }

    for (int stat = 270; stat <= 289; ++stat)
    {
        StatSet(stat, 1);
    }
}

void UnlockRank()
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

        UnlockChallengeTable(challengeTable, challengeCount);
    }
}

void Cmd_UnlockStats_f()
{
    int weaponCount = 0;
    int itemCount = 0;
    int challengeCount = 0;

    UnlockRank();
    UnlockItems(weaponCount, itemCount);
    UnlockChallenges(challengeCount);

    ExecuteCommand("updategamerprofile");
}
} // namespace

stats::stats()
{
    command::add("unlockstats", Cmd_UnlockStats_f);
}

stats::~stats()
{
}

} // namespace mp
} // namespace iw3
