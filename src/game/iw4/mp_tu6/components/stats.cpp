#include "pch.h"
#include <stdlib.h>
#include "stats.h"

namespace iw4
{
namespace mp_tu6
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

    const StringTableCell &cell = table->values[row * table->columnCount + column];
    return cell.string ? cell.string : "";
}

void ExecuteCommand(const char *command)
{
    char commandLine[1024] = {};
    const size_t commandLength = std::strlen(command);

    if (commandLength >= sizeof(commandLine) - 1)
    {
        Com_Printf(0, "[codxe][IW4 TU6] unlockstats: command too long: %s\n", command);
        return;
    }

    std::memcpy(commandLine, command, commandLength);
    commandLine[commandLength] = '\n';

    Cmd_ExecuteSingleCommand(0, 0, commandLine);
}

void Cmd_UnlockStats_f()
{
    if (clientUIActives[0].cgameInitialized)
    {
        Com_Printf(0, "[codxe][IW4 TU6] unlockstats: not allowed while ingame\n");
        return;
    }

    ExecuteCommand("setPlayerData prestige 10");
    ExecuteCommand("setPlayerData experience 2516000");
    ExecuteCommand("setPlayerData iconUnlocked cardicon_prestige10_02 1");

    const XAssetHeader challengeAsset = DB_FindXAssetHeader(ASSET_TYPE_STRINGTABLE, "mp/allchallengestable.csv");
    const StringTable *challengeTable = challengeAsset.stringTable;

    if (!challengeTable)
    {
        Com_Printf(0, "[codxe][IW4 TU6] unlockstats: mp/allchallengestable.csv not found\n");
        return;
    }

    int challengeCount = 0;
    for (int row = 0; row < challengeTable->rowCount; ++row)
    {
        const char *challenge = TableLookup(challengeTable, row, 0);
        if (!challenge || !*challenge)
        {
            continue;
        }

        int maxState = 0;
        int maxProgress = 0;

        for (int tier = 0; tier < 10; ++tier)
        {
            const int progress = atoi(TableLookup(challengeTable, row, 6 + tier * 2));
            if (!progress)
            {
                break;
            }

            maxState = tier + 2;
            maxProgress = progress;
        }

        ExecuteCommand(va("setPlayerData challengeState %s %d", challenge, maxState));
        ExecuteCommand(va("setPlayerData challengeProgress %s %d", challenge, maxProgress));
        ++challengeCount;
    }

    Com_Printf(0, "[codxe][IW4 TU6] unlockstats: queued stats and %i challenge rows\n", challengeCount);
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

stats::stats()
{
}

stats::~stats()
{
}

} // namespace mp_tu6
} // namespace iw4
