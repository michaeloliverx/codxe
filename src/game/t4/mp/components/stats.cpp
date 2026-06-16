#include "pch.h"
#include <cctype>
#include <stdlib.h>
#include "events.h"
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
                   [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
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

void RegisterCommands()
{
    Cmd_AddCommandInternal("unlockstats", Cmd_UnlockStats_f, &Cmd_UnlockStats_VAR);
}
} // namespace

stats::stats()
{
    Events::OnCmdInit(RegisterCommands);
}

stats::~stats()
{
}

} // namespace mp
} // namespace t4
