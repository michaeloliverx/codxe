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
Detour CL_CheckForResend_Detour;

Detour ClassEditorInput_Detour;
Detour ClassEditorGetStat_Detour;
Detour ClassEditorSetStat_Detour;
Detour ClassEditorCommand_Detour;
Detour ClassEditorQueue_Detour;
Detour ClassEditorKeyboardComplete_Detour;

struct ClassEditor
{
    UiContext *context;
    menuDef_t *popup;
    int controller;
    int previousProfile;
};

ClassEditor classEditor = {};
int inputController = -1;

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

    return filesystem::JoinPath("codxe:\\players", filename);
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

bool RestoreEmptyCustomClasses(unsigned int controllerIndex)
{
    const unsigned __int8 *stats = controllerStatData[controllerIndex].playerStats + 4;
    const StringTable *classTable = nullptr;
    bool restored = false;
    for (int base = 200; base < 250; base += 10)
    {
        // An empty slot has no class data and no Create-a-Class initialization marker.
        if (!std::all_of(stats + base, stats + base + 10, [](unsigned __int8 value) { return value == 0; }))
            continue;

        if (!classTable)
        {
            classTable = FindStringTable("mp/classtable.csv");
            if (!classTable)
                break;
        }

        const int fieldCount = 9;
        int values[fieldCount] = {};
        int count = 0;
        for (int row = 0; row < classTable->rowCount; ++row)
        {
            int stat;
            int value;
            if (TryParseInt(TableLookup(classTable, row, 1), &stat) && stat > base && stat <= base + fieldCount &&
                TryParseInt(TableLookup(classTable, row, 5), &value) && value >= 0 && value < 256)
            {
                values[stat - base - 1] = value;
                ++count;
            }
        }

        if (count != fieldCount || !values[0])
            continue;

        for (int i = 0; i < fieldCount; ++i)
            StatSet(controllerIndex, base + 1 + i, values[i]);

        restored = true;
    }
    return restored;
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

bool IsSignedInToLive(unsigned int controllerIndex)
{
    return XUserGetSigninState(controllerIndex) == eXUserSigninState_SignedInToLive;
}

bool IsOfflineGame()
{
    return Dvar_GetBool("systemlink") || Dvar_GetBool("splitscreen");
}

bool IsMenuOnStack(const UiContext *dc, const menuDef_t *menu)
{
    if (!menu)
        return false;

    for (int i = dc->openMenuCount - 1; i >= 0; --i)
    {
        if (dc->menuStack[i] == menu)
            return true;
    }
    return false;
}

int GetClassEditorController()
{
    if (classEditor.context && clientUIActives[0].connectionState == CA_DISCONNECTED &&
        IsMenuOnStack(classEditor.context, classEditor.popup))
        return classEditor.controller;

    return -1;
}

void FinishClassEditIfClosed()
{
    if (!classEditor.context || GetClassEditorController() >= 0)
        return;

    const int controller = classEditor.controller;
    const int previousProfile = classEditor.previousProfile;
    classEditor = ClassEditor();
    if (controllerStatData[controller].statWriteNeeded)
        SaveStats(controller);
    if (previousProfile != controller)
        GamerProfile_UpdateDvarsFromProfile(previousProfile);
}

void ClassEditorInput_Hook(int localClientNum, int controllerIndex, int key, int down, unsigned int time)
{
    FinishClassEditIfClosed();
    const int editor = GetClassEditorController();
    if (editor >= 0 && down && editor != controllerIndex)
        return;

    const int previousInput = inputController;
    inputController = down ? controllerIndex : -1;
    ClassEditorInput_Detour.GetOriginal<decltype(CL_GamepadButtonEvent)>()(localClientNum, controllerIndex, key, down,
                                                                        time);
    inputController = previousInput;
    FinishClassEditIfClosed();
}

int ClassEditorGetStat_Hook(int controllerIndex, int stat)
{
    // TU4 stat(), statRangeAnyBitsSet(), and the two menu stat bitmask commands.
    const auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
    if (caller == 0x821F6D20 || caller == 0x821F6C6C || caller == 0x821E7B8C || caller == 0x821E7C0C)
    {
        const int editor = GetClassEditorController();
        if (editor >= 0)
            controllerIndex = editor;
    }
    return ClassEditorGetStat_Detour.GetOriginal<decltype(LiveStorage_GetStat)>()(controllerIndex, stat);
}

void ClassEditorSetStat_Hook(int controllerIndex, int stat, unsigned int value)
{
    // TU4 menu statClearBitMask, statSetBitMask, and statSetUsingTable.
    const auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
    if (caller == 0x821E7BA0 || caller == 0x821E7C20 || caller == 0x821E7E90)
    {
        const int editor = GetClassEditorController();
        if (editor >= 0)
            controllerIndex = editor;
    }
    ClassEditorSetStat_Detour.GetOriginal<LiveStorage_SetStat_t>()(controllerIndex, stat, value);
}

int ClassEditorCommand_Hook(int localClientNum, int controllerIndex, const char *command)
{
    const auto execute = ClassEditorCommand_Detour.GetOriginal<decltype(Cmd_ExecuteSingleCommand)>();
    const int editor = localClientNum == 0 ? GetClassEditorController() : -1;
    if (editor < 0)
        return execute(localClientNum, controllerIndex, command);

    if (Equals(command, "updategamerprofile"))
    {
        // The stock command defaults to the last input port unless passed an explicit controller.
        char update[] = "updategamerprofile 0";
        update[sizeof(update) - 2] += static_cast<char>(editor);
        return execute(localClientNum, editor, update);
    }

    if (I_strnicmp(command, "statset ", 8) == 0 || I_strnicmp(command, "statGetInDvar ", 14) == 0 ||
        I_strnicmp(command, "ui_keyboard ", 12) == 0 || Equals(command, "uploadstats"))
        controllerIndex = editor;

    return execute(localClientNum, controllerIndex, command);
}

void ClassEditorQueue_Hook(int localClientNum, const char *text)
{
    const int editor = localClientNum == 0 ? GetClassEditorController() : -1;
    // The stock slot popup queues these after opening the editor. Bind the write before the
    // popup can close in the same frame and leave the command running under the lobby profile.
    if (editor >= 0 && (Equals(text, "statset 210 1\n") || Equals(text, "statset 220 1\n") ||
                        Equals(text, "statset 230 1\n") || Equals(text, "statset 240 1\n")))
    {
        Cmd_ExecuteSingleCommand(localClientNum, editor, text);
        return;
    }
    ClassEditorQueue_Detour.GetOriginal<decltype(Cbuf_AddText)>()(localClientNum, text);
}

void *ClassEditorKeyboardComplete_Hook(int controllerIndex)
{
    const int editor = GetClassEditorController();
    return ClassEditorKeyboardComplete_Detour.GetOriginal<decltype(CL_GuideKeyboardComplete)>()(
        editor >= 0 ? editor : controllerIndex);
}

void CL_CheckForResend_Hook(int localClientNum)
{
    // The stock sender snapshots controller stats here, before the class menu opens.
    if (IsValidControllerIndex(localClientNum) && clientUIActives[localClientNum].connectionState == CA_SENDINGSTATS)
        Stats::EnsureOfflineStats(localClientNum);

    CL_CheckForResend_Detour.GetOriginal<decltype(CL_CheckForResend)>()(localClientNum);
}

void LiveStorage_ReadStats_Hook(unsigned int controllerIndex)
{
    if (!IsSignedInToLive(controllerIndex))
    {
        LoadOrInitializeStats(controllerIndex);
        return;
    }

    LiveStorage_ReadStats_Detour.GetOriginal<LiveStorage_ReadStats_t>()(controllerIndex);
}

void LiveStorage_UploadStats_Hook(unsigned int controllerIndex)
{
    if (IsOfflineGame() || GetClassEditorController() == static_cast<int>(controllerIndex))
    {
        SaveStats(controllerIndex);
        return;
    }

    LiveStorage_UploadStats_Detour.GetOriginal<LiveStorage_UploadStats_t>()(controllerIndex);
}
} // namespace

void Stats::BeginSplitScreenClassEdit(UiContext *dc)
{
    FinishClassEditIfClosed();
    if (classEditor.context || dc->localClientNum != 0 || clientUIActives[0].connectionState != CA_DISCONNECTED ||
        !IsMenuOnStack(dc, Menus_FindByName(dc, "menu_gamesetup_splitscreen")))
        return;

    const int controller = inputController >= 0 ? inputController : CL_ControllerIndexFromClientNum(0);
    auto *popup = Menus_FindByName(dc, "live_cac_popup");
    if (!popup || !IsValidControllerIndex(controller))
        return;

    if (!IsSignedInToLive(controller) && !controllerStatData[controller].statsFetched)
        LoadOrInitializeStats(controller);

    const int previousProfile = *cl_controller_in_use;
    classEditor.context = dc;
    classEditor.popup = popup;
    classEditor.controller = controller;
    classEditor.previousProfile = IsValidControllerIndex(previousProfile) ? previousProfile : 0;
    GamerProfile_UpdateDvarsFromProfile(controller);
}

void Stats::EnsureOfflineStats(int localClientNum)
{
    if (localClientNum < 0 || localClientNum >= 4 || !Dvar_GetBool("splitscreen") || Dvar_GetBool("onlinegame"))
        return;

    const int controllerIndex = CL_ControllerIndexFromClientNum(localClientNum);
    if (!IsValidControllerIndex(controllerIndex) || IsSignedInToLive(controllerIndex))
        return;

    if (!controllerStatData[controllerIndex].statsFetched)
        LoadOrInitializeStats(controllerIndex);

    if (RestoreEmptyCustomClasses(controllerIndex))
        SaveStats(controllerIndex);
}

void Stats::LoadStatsScript(int localClientNum, const char ** /*args*/)
{
    const int controllerIndex = CL_ControllerIndexFromClientNum(localClientNum);
    if (!IsValidControllerIndex(controllerIndex))
    {
        Com_PrintError(CON_CHANNEL_DONT_FILTER, "[codxe][IW3][Stats] Invalid controller %i for local client %i\n",
                       controllerIndex, localClientNum);
        return;
    }

    // Xbox Live profiles keep their Live stats: offline stats (unlocked on first use) are only for profiles that
    // are not signed in to Live, as in LiveStorage_ReadStats_Hook
    if (IsSignedInToLive(controllerIndex))
        return;

    LoadOrInitializeStats(controllerIndex);
}

Stats::Stats()
{
    command::add("unlockstats", Cmd_UnlockStats_f);

    LiveStorage_ReadStats_Detour = Detour(LiveStorage_ReadStats, LiveStorage_ReadStats_Hook);
    LiveStorage_ReadStats_Detour.Install();

    LiveStorage_UploadStats_Detour = Detour(LiveStorage_UploadStats, LiveStorage_UploadStats_Hook);
    LiveStorage_UploadStats_Detour.Install();

    CL_CheckForResend_Detour = Detour(CL_CheckForResend, CL_CheckForResend_Hook);
    CL_CheckForResend_Detour.Install();

    ClassEditorInput_Detour = Detour(CL_GamepadButtonEvent, ClassEditorInput_Hook);
    ClassEditorInput_Detour.Install();
    ClassEditorGetStat_Detour = Detour(LiveStorage_GetStat, ClassEditorGetStat_Hook);
    ClassEditorGetStat_Detour.Install();
    ClassEditorSetStat_Detour = Detour(LiveStorage_SetStat, ClassEditorSetStat_Hook);
    ClassEditorSetStat_Detour.Install();
    ClassEditorCommand_Detour = Detour(Cmd_ExecuteSingleCommand, ClassEditorCommand_Hook);
    ClassEditorCommand_Detour.Install();
    ClassEditorQueue_Detour = Detour(Cbuf_AddText, ClassEditorQueue_Hook);
    ClassEditorQueue_Detour.Install();
    ClassEditorKeyboardComplete_Detour = Detour(CL_GuideKeyboardComplete, ClassEditorKeyboardComplete_Hook);
    ClassEditorKeyboardComplete_Detour.Install();
}
} // namespace mp
} // namespace iw3
