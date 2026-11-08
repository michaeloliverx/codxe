#include "pch.h"
#include "mods.h"

#include "fastfiles.h"
#include "usermaps.h"

namespace t4
{
namespace mp
{
namespace
{
const float MODS_FEEDER_ID = 9.0f;
const char *const FS_GAME_PREFIX = "mods/";

struct ModEntry
{
    std::string name;
    std::string displayName;
};

std::vector<ModEntry> mods;
int selectedMod = 0;
dvar_s *fs_game = nullptr;

Detour UI_FeederCount_Detour;
Detour UI_FeederItemText_Detour;
Detour UI_FeederSelection_Detour;
Detour Cmd_ExecFromDisk_Detour;

bool IsSafeModName(const char *name)
{
    if (!name || !*name || std::strcmp(name, ".") == 0 || std::strcmp(name, "..") == 0)
        return false;

    for (const char *cursor = name; *cursor; ++cursor)
    {
        const unsigned char c = static_cast<unsigned char>(*cursor);
        if (c < ' ' || c == '/' || c == '\\' || c == ':')
            return false;
    }

    return true;
}

std::string ReadDisplayName(const std::string &directory, const std::string &fallback)
{
    std::string text = filesystem::ReadFileToString(filesystem::JoinPath(directory.c_str(), "description.txt"));
    const size_t lineEnd = text.find_first_of("\r\n");
    if (lineEnd != std::string::npos)
        text.erase(lineEnd);

    const size_t first = text.find_first_not_of(" \t");
    const size_t last = text.find_last_not_of(" \t");
    return first == std::string::npos ? fallback : text.substr(first, last - first + 1);
}

void ScanMods(int localClientNum)
{
    mods.clear();
    selectedMod = 0;

    const std::string pattern = filesystem::JoinPath(MOD_DIR, "*");
    WIN32_FIND_DATAA data;
    HANDLE handle = FindFirstFileA(pattern.c_str(), &data);
    if (handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !IsSafeModName(data.cFileName))
                continue;

            ModEntry entry;
            entry.name = data.cFileName;
            entry.displayName = ReadDisplayName(filesystem::JoinPath(MOD_DIR, data.cFileName), entry.name);
            mods.push_back(entry);
        } while (FindNextFileA(handle, &data));
        FindClose(handle);
    }

    std::sort(mods.begin(), mods.end(), [](const ModEntry &left, const ModEntry &right)
              { return _stricmp(left.displayName.c_str(), right.displayName.c_str()) < 0; });

    Dvar_SetIntByName("ui_modCount", static_cast<int>(mods.size()));

    // Reset both the native listbox cursor and scroll position when rescanning.
    Menu_SetFeederSelection(UI_GetContext(localClientNum), nullptr, static_cast<int>(MODS_FEEDER_ID), 0, "mods_menu");
    DbgPrint("[codxe][T4][ModList] Found %u mod(s) in %s\n", static_cast<unsigned int>(mods.size()), MOD_DIR);
}

bool CanChangeMod()
{
    // Changing database assets or script roots during a match would leave stale pointers.
    return fs_game && !Dvar_GetBool("cl_ingame") && !Dvar_GetBool("sv_running");
}

void RunMod(int localClientNum)
{
    if (!CanChangeMod() || selectedMod < 0 || selectedMod >= static_cast<int>(mods.size()))
        return;

    const std::string name = mods[selectedMod].name;
    const std::string directory = filesystem::JoinPath(MOD_DIR, name.c_str());
    if (!filesystem::DirectoryExists(directory.c_str()))
        return;

    const std::string value = std::string(FS_GAME_PREFIX) + name;
    Dvar_SetStringByName("fs_game", value.c_str());
    FastFiles::ReloadModZone();
    Cbuf_AddText(localClientNum, "exec mod.cfg\n");
    DbgPrint("[codxe][T4][ModList] Activated %s\n", value.c_str());
}

void ClearMods()
{
    if (!CanChangeMod())
        return;

    Dvar_SetStringByName("fs_game", "");
    FastFiles::ReloadModZone();
    DbgPrint("[codxe][T4][ModList] Cleared active mod\n");
}

int UI_FeederCount_Hook(int localClientNum, float feederID)
{
    if (feederID == Usermaps::FEEDER_ID)
        return Usermaps::Count();
    if (feederID == MODS_FEEDER_ID)
        return static_cast<int>(mods.size());

    return UI_FeederCount_Detour.GetOriginal<decltype(UI_FeederCount)>()(localClientNum, feederID);
}

const char *UI_FeederItemText_Hook(int localClientNum, itemDef_s *item, float feederID, int index, unsigned int column,
                                   Material **handle)
{
    if (feederID == Usermaps::FEEDER_ID)
    {
        if (handle)
            *handle = nullptr;
        return Usermaps::Text(index);
    }
    if (feederID == MODS_FEEDER_ID)
    {
        if (handle)
            *handle = nullptr;
        return index >= 0 && index < static_cast<int>(mods.size()) ? mods[index].displayName.c_str() : "";
    }

    return UI_FeederItemText_Detour.GetOriginal<decltype(UI_FeederItemText)>()(localClientNum, item, feederID, index,
                                                                               column, handle);
}

void UI_FeederSelection_Hook(int localClientNum, float feederID, int index)
{
    if (feederID == Usermaps::FEEDER_ID)
    {
        Usermaps::Select(index);
        return;
    }
    if (feederID == MODS_FEEDER_ID)
    {
        if (index >= 0 && index < static_cast<int>(mods.size()))
            selectedMod = index;
        return;
    }

    UI_FeederSelection_Detour.GetOriginal<decltype(UI_FeederSelection)>()(localClientNum, feederID, index);
}

bool Cmd_ExecFromDisk_Hook(int localClientNum, int controllerIndex, const char *filename)
{
    const std::string path = ModList::ResolvePath(filename);
    if (!path.empty() && filesystem::FileExists(path.c_str()))
    {
        const std::string contents = filesystem::ReadFileToString(path);
        Cbuf_ExecuteBuffer(localClientNum, controllerIndex, contents.c_str());
        return true;
    }

    return Cmd_ExecFromDisk_Detour.GetOriginal<decltype(Cmd_ExecFromDisk)>()(localClientNum, controllerIndex, filename);
}
} // namespace

void ModList::OnDvarInit()
{
    fs_game = Dvar_RegisterString("fs_game", "", DVAR_ARCHIVE, "The active mod directory");
}

std::string ModList::GetActiveName()
{
    if (!fs_game || !fs_game->current.string)
        return std::string();

    const char *value = fs_game->current.string;
    const size_t prefixLength = std::strlen(FS_GAME_PREFIX);
    if (_strnicmp(value, FS_GAME_PREFIX, prefixLength) != 0)
        return std::string();

    const char *name = value + prefixLength;
    return IsSafeModName(name) ? name : std::string();
}

std::string ModList::ResolvePath(const char *relativePath)
{
    const std::string name = GetActiveName();
    if (name.empty() || !relativePath || !*relativePath)
        return std::string();

    const std::string directory = filesystem::JoinPath(MOD_DIR, name.c_str());
    return filesystem::JoinPath(directory.c_str(), relativePath);
}

bool ModList::RunMenuScript(int localClientNum, const char *command)
{
    if (_stricmp(command, "LoadMods") == 0)
        ScanMods(localClientNum);
    else if (_stricmp(command, "RunMod") == 0)
        RunMod(localClientNum);
    else if (_stricmp(command, "ClearMods") == 0)
        ClearMods();
    else
        return false;

    return true;
}

ModList::ModList()
{
    UI_FeederCount_Detour = Detour(UI_FeederCount, UI_FeederCount_Hook);
    UI_FeederCount_Detour.Install();
    UI_FeederItemText_Detour = Detour(UI_FeederItemText, UI_FeederItemText_Hook);
    UI_FeederItemText_Detour.Install();
    UI_FeederSelection_Detour = Detour(UI_FeederSelection, UI_FeederSelection_Hook);
    UI_FeederSelection_Detour.Install();
    Cmd_ExecFromDisk_Detour = Detour(Cmd_ExecFromDisk, Cmd_ExecFromDisk_Hook);
    Cmd_ExecFromDisk_Detour.Install();
}

ModList::~ModList()
{
    Cmd_ExecFromDisk_Detour.Remove();
    UI_FeederSelection_Detour.Remove();
    UI_FeederItemText_Detour.Remove();
    UI_FeederCount_Detour.Remove();
    mods.clear();
    selectedMod = 0;
    fs_game = nullptr;
}
} // namespace mp
} // namespace t4
