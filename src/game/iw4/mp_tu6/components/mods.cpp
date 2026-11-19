#include "pch.h"
#include "mods.h"
#include "fastfiles.h"
#include "ui_feeder.h"
#include "ui_script.h"
#include "usermap_preview.h"
#include "common/mod_metadata.h"

namespace iw4
{
namespace mp_tu6
{
namespace
{
const int MODS_FEEDER = 9;
const char *const MODS_DIRECTORY = "codxe:\\mods";
std::vector<mod_metadata::Entry> mods;
int selected[4] = {};
dvar_t *fs_game = nullptr;
Detour Cmd_ExecFromDisk_Detour;

bool IsSafeModName(const char *name)
{
    if (!name || !*name || std::strcmp(name, ".") == 0 || std::strcmp(name, "..") == 0)
        return false;
    for (const char *c = name; *c; ++c)
        if (static_cast<unsigned char>(*c) < ' ' || *c == '/' || *c == '\\' || *c == ':')
            return false;
    return true;
}

int Count()
{
    return static_cast<int>(mods.size());
}

const char *Text(int client, int index, int column)
{
    if (index < 0 || index >= Count())
        return "";
    if (column == 0)
        return mods[index].displayName.c_str();
    if (column == 1 && client >= 0 && client < ARRAYSIZE(selected) && selected[client] == index)
        return "\x01"; // Stock A-button glyph, matching the custom map list.
    return "";
}

void UpdateSelection(int index)
{
    const mod_metadata::Entry *entry = index >= 0 && index < Count() ? &mods[index] : nullptr;
    Dvar_SetStringByName("ui_codxe_mod_name", entry ? entry->displayName.c_str() : "");
    Dvar_SetStringByName("ui_codxe_mod_description", entry ? entry->description.c_str() : "");
    Dvar_SetStringByName("ui_codxe_mod_author", entry ? entry->credits.author.c_str() : "");
    Dvar_SetStringByName("ui_codxe_mod_revision", entry ? entry->credits.revision.c_str() : "");
    Dvar_SetStringByName("ui_codxe_mod_has_preview", entry && UsermapPreview::LoadMod(entry->previewPath) ? "1" : "0");
}

void Select(int client, int index)
{
    if (client >= 0 && client < ARRAYSIZE(selected) && index >= 0 && index < Count() && index != selected[client])
    {
        selected[client] = index;
        UpdateSelection(index);
    }
}

void Load(int client, const char **)
{
    mods.clear();
    memset(selected, 0, sizeof(selected));
    const std::string pattern = filesystem::JoinPath(MODS_DIRECTORY, "*");
    WIN32_FIND_DATAA data;
    HANDLE search = FindFirstFileA(pattern.c_str(), &data);
    if (search != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && IsSafeModName(data.cFileName))
            {
                const std::string directory = filesystem::JoinPath(MODS_DIRECTORY, data.cFileName);
                mods.push_back(mod_metadata::Read(directory.c_str(), data.cFileName));
            }
        } while (FindNextFileA(search, &data));
        FindClose(search);
    }
    std::sort(mods.begin(), mods.end(), [](const mod_metadata::Entry &left, const mod_metadata::Entry &right)
              { return left.displayName < right.displayName; });
    Dvar_SetStringByName("ui_modCount", va("%d", Count()));
    Menu_SetFeederSelection(UI_GetClientDC(client), nullptr, MODS_FEEDER, 0, "mods_menu");
    UpdateSelection(0);
}

bool CanChangeMod()
{
    // Database assets and script roots may only change outside a running match.
    return fs_game && !Dvar_GetBool("cl_ingame") && !Dvar_GetBool("sv_running");
}

void RefreshMenus(int client)
{
    // Re-resolve menus after loading or removing a mod's menu overrides.
    UI_LoadMenuLists(client);
    Menus_OpenByName(UI_GetClientDC(client), "main");
}

void Run(int client, const char **)
{
    if (!CanChangeMod() || client < 0 || client >= ARRAYSIZE(selected))
        return;
    const int index = selected[client];
    if (index < 0 || index >= Count())
        return;
    const std::string &folder = mods[index].name;
    const std::string path = filesystem::JoinPath(MODS_DIRECTORY, folder.c_str());
    if (!filesystem::DirectoryExists(path.c_str()))
        return;
    Dvar_SetString(fs_game, ("mods/" + folder).c_str());
    fastfiles::ReloadModZone();
    RefreshMenus(client);
    Cbuf_AddText(client, "exec mod.cfg\n");
}

void Clear(int client, const char **)
{
    if (!CanChangeMod())
        return;
    Dvar_SetString(fs_game, "");
    fastfiles::ReloadModZone();
    RefreshMenus(client);
}

bool Exec(int client, int controller, const char *filename)
{
    const std::string path = ModList::ResolvePath(filename);
    if (!path.empty() && filesystem::FileExists(path.c_str()))
    {
        const std::string contents = filesystem::ReadFileToString(path);
        Cbuf_ExecuteBuffer(client, controller, contents.c_str());
        return true;
    }
    return Cmd_ExecFromDisk_Detour.GetOriginal<Cmd_ExecFromDisk_t>()(client, controller, filename);
}
} // namespace

void ModList::OnDvarInit()
{
    fs_game = Dvar_RegisterString("fs_game", "", 1, "The active mod directory");
    Dvar_RegisterString("ui_modCount", "0", 0, "Number of installed mods");
    const char *names[] = {"ui_codxe_mod_name", "ui_codxe_mod_description", "ui_codxe_mod_author",
                           "ui_codxe_mod_revision", "ui_codxe_mod_has_preview"};
    for (size_t i = 0; i < ARRAYSIZE(names); ++i)
        Dvar_RegisterString(names[i], "", 0, "Selected mod metadata");
}

std::string ModList::GetActiveName()
{
    if (!fs_game || !fs_game->current.string || _strnicmp(fs_game->current.string, "mods/", 5) != 0)
        return "";
    const char *name = fs_game->current.string + 5;
    return IsSafeModName(name) ? name : "";
}

std::string ModList::ResolvePath(const char *relativePath)
{
    const std::string name = GetActiveName();
    if (name.empty() || !relativePath || !*relativePath || *relativePath == '/' || *relativePath == '\\' ||
        std::strchr(relativePath, ':') || std::strstr(relativePath, ".."))
        return "";
    const std::string directory = filesystem::JoinPath(MODS_DIRECTORY, name.c_str());
    return filesystem::JoinPath(directory.c_str(), relativePath);
}

ModList::ModList()
{
    const UIFeeder::Callbacks callbacks = {Count, Text, Select};
    UIFeeder::Add(static_cast<float>(MODS_FEEDER), callbacks);
    UIScript::Add("LoadMods", Load);
    UIScript::Add("RunMod", Run);
    UIScript::Add("ClearMods", Clear);
    Cmd_ExecFromDisk_Detour = Detour(Cmd_ExecFromDisk, Exec);
    Cmd_ExecFromDisk_Detour.Install();
}
} // namespace mp_tu6
} // namespace iw4
