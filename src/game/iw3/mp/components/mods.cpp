#include "pch.h"
#include "mods.h"

#include "fastfiles.h"
#include "ui_feeder.h"

namespace iw3
{
namespace mp
{
namespace
{
const float MODS_FEEDER_ID = 9.0f;
const char *const MODS_DIRECTORY = "game:\\_codxe\\mods";
const char *const FS_GAME_PREFIX = "mods/";

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

std::string BuildFsGame(const char *name)
{
    return std::string(FS_GAME_PREFIX) + name;
}

std::string ReadDisplayName(const std::string &path, const std::string &fallback)
{
    std::string displayName = filesystem::ReadFileToString(path);
    const size_t lineEnd = displayName.find_first_of("\r\n");
    if (lineEnd != std::string::npos)
        displayName.erase(lineEnd);

    size_t first = 0;
    while (first < displayName.length() && static_cast<unsigned char>(displayName[first]) <= ' ')
        ++first;

    size_t last = displayName.length();
    while (last > first && static_cast<unsigned char>(displayName[last - 1]) <= ' ')
        --last;

    return first == last ? fallback : displayName.substr(first, last - first);
}
} // namespace

std::vector<FeederEntry> ModList::Mods;
int ModList::CurrentMod = 0;
bool ModList::ModsScanned = false;
dvar_s *ModList::fs_game = nullptr;

ModList::ModList()
{
    UIFeeder::Add(MODS_FEEDER_ID, GetItemCount, GetItemText, Select);
}

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
    if (I_strnicmp(value, FS_GAME_PREFIX, static_cast<int>(prefixLength)) != 0)
        return std::string();

    const char *name = value + prefixLength;
    return IsSafeModName(name) ? name : std::string();
}

std::string ModList::GetActivePath()
{
    const std::string name = GetActiveName();
    return name.empty() ? std::string() : filesystem::JoinPath(MODS_DIRECTORY, name.c_str());
}

std::string ModList::ResolvePath(const char *relativePath)
{
    const std::string activePath = GetActivePath();
    if (activePath.empty() || !relativePath || !*relativePath)
        return std::string();

    return filesystem::JoinPath(activePath.c_str(), relativePath);
}

bool ModList::RunMod(const char *name)
{
    if (!fs_game || !IsSafeModName(name))
        return false;

    const std::string modPath = filesystem::JoinPath(MODS_DIRECTORY, name);
    if (!filesystem::DirectoryExists(modPath.c_str()))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Mod directory does not exist: %s\n", modPath.c_str());
        return false;
    }

    const std::string fsGame = BuildFsGame(name);
    DbgPrint("[codxe][IW3][ModList] Activating %s\n", fsGame.c_str());
    Dvar_SetStringFromSource(fs_game, fsGame.c_str(), DVAR_SOURCE_INTERNAL);
    FastFiles::ReloadModZone();
    Cbuf_AddText(0, "exec mod.cfg\n");
    return true;
}

void ModList::ClearMods()
{
    if (!fs_game)
        return;

    DbgPrint("[codxe][IW3][ModList] Clearing %s\n",
             fs_game->current.string && *fs_game->current.string ? fs_game->current.string : "<none>");
    Dvar_SetStringFromSource(fs_game, "", DVAR_SOURCE_INTERNAL);
    FastFiles::ReloadModZone();
}

void ModList::ScanMods()
{
    std::string selectedName;
    if (CurrentMod >= 0 && CurrentMod < static_cast<int>(Mods.size()))
        selectedName = Mods[CurrentMod].name;

    Mods.clear();
    CurrentMod = 0;
    ModsScanned = true;
    UIFeeder::SetSelectedIndex(MODS_FEEDER_ID, CurrentMod);

    const std::string searchPattern = filesystem::JoinPath(MODS_DIRECTORY, "*");
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(searchPattern.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        DbgPrint("[codxe][IW3][ModList] Mod directory is unavailable: %s\n", MODS_DIRECTORY);
        return;
    }

    do
    {
        const std::string name = findData.cFileName;
        if (name == "." || name == ".." || (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            continue;

        FeederEntry entry;
        entry.name = name;
        const std::string directory = filesystem::JoinPath(MODS_DIRECTORY, name.c_str());
        entry.displayName = ReadDisplayName(filesystem::JoinPath(directory.c_str(), "description.txt"), name);
        Mods.push_back(entry);
    } while (FindNextFileA(findHandle, &findData) != 0);

    FindClose(findHandle);
    std::sort(Mods.begin(), Mods.end(), [](const FeederEntry &left, const FeederEntry &right)
              { return I_stricmp(left.displayName.c_str(), right.displayName.c_str()) < 0; });

    for (size_t i = 0; i < Mods.size(); ++i)
    {
        if (I_stricmp(Mods[i].name.c_str(), selectedName.c_str()) == 0)
        {
            CurrentMod = static_cast<int>(i);
            break;
        }
    }

    UIFeeder::SetSelectedIndex(MODS_FEEDER_ID, CurrentMod);

    DbgPrint("[codxe][IW3][ModList] Found %u mod(s) in %s\n", static_cast<unsigned int>(Mods.size()), MODS_DIRECTORY);
}

void ModList::EnsureModsScanned()
{
    if (!ModsScanned)
        ScanMods();
}

int ModList::GetItemCount()
{
    EnsureModsScanned();
    return static_cast<int>(Mods.size());
}

const char *ModList::GetItemText(int index)
{
    EnsureModsScanned();
    return index >= 0 && index < static_cast<int>(Mods.size()) ? Mods[index].displayName.c_str() : "";
}

void ModList::Select(int index)
{
    EnsureModsScanned();
    if (index < 0 || index >= static_cast<int>(Mods.size()))
        return;

    CurrentMod = index;
    DbgPrint("[codxe][IW3][ModList] Selected mod: %s\n", Mods[CurrentMod].name.c_str());
}

void ModList::LoadModsScript(int /*localClientNum*/, const char ** /*args*/)
{
    ScanMods();
}

void ModList::RunModScript(int /*localClientNum*/, const char ** /*args*/)
{
    EnsureModsScanned();
    if (CurrentMod >= 0 && CurrentMod < static_cast<int>(Mods.size()))
        RunMod(Mods[CurrentMod].name.c_str());
}

void ModList::ClearModsScript(int /*localClientNum*/, const char ** /*args*/)
{
    ClearMods();
}

} // namespace mp
} // namespace iw3
