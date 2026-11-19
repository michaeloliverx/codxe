#include "pch.h"
#include "mods.h"

#include "fastfiles.h"
#include "ui_feeder.h"
#include "image/usermap_preview.h"

namespace iw3
{
namespace mp
{
namespace
{
const float MODS_FEEDER_ID = 9.0f;
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

void SetMetadata(const char *name, const char *value)
{
    dvar_s *dvar = Dvar_FindMalleableVar(name);
    if (dvar)
        Dvar_SetStringFromSource(dvar, value, DVAR_SOURCE_INTERNAL);
}
int MeasureModTitleWrapHeight(const char *title)
{
    // Match the font, scale, and width of mod_title in the IW3 menu.
    const float scale = 0.4f;
    const unsigned int width = 267;
    Font_s *font = UI_GetFontHandle(1, scale);
    if (!font || font->pixelHeight <= 0)
        return 0;

    const float drawScale = scale * 48.0f / font->pixelHeight;
    const int lineHeight = static_cast<int>(scale * 48.0f) + 5;
    int extraHeight = 0;
    while (*title)
    {
        const char *next = R_TextLineWrapPosition(title, 1024, width, font, drawScale);
        if (!next || next <= title)
            break;
        while (std::isspace(static_cast<unsigned char>(*next)))
            ++next;
        if (!*next)
            break;
        extraHeight += lineHeight;
        title = next;
    }
    return extraHeight;
}
} // namespace

std::vector<mod_metadata::Entry> ModList::Mods;
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
    const char *names[] = {"ui_codxe_mod_name", "ui_codxe_mod_description", "ui_codxe_mod_author",
                           "ui_codxe_mod_revision", "ui_codxe_mod_has_preview", "ui_codxe_mod_title_wrap_height"};
    for (size_t i = 0; i < ARRAYSIZE(names); ++i)
        Dvar_RegisterString(names[i], "", DVAR_FLAG_NONE, "Selected mod metadata");
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
    return name.empty() ? std::string() : filesystem::JoinPath(MOD_DIR, name.c_str());
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

    const std::string modPath = filesystem::JoinPath(MOD_DIR, name);
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

    const std::string searchPattern = filesystem::JoinPath(MOD_DIR, "*");
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(searchPattern.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        DbgPrint("[codxe][IW3][ModList] Mod directory is unavailable: %s\n", MOD_DIR);
        return;
    }

    do
    {
        const std::string name = findData.cFileName;
        if (name == "." || name == ".." || (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            continue;

        const std::string directory = filesystem::JoinPath(MOD_DIR, name.c_str());
        Mods.push_back(mod_metadata::Read(directory.c_str(), name.c_str()));
    } while (FindNextFileA(findHandle, &findData) != 0);

    FindClose(findHandle);
    std::sort(Mods.begin(), Mods.end(), [](const mod_metadata::Entry &left, const mod_metadata::Entry &right)
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

    DbgPrint("[codxe][IW3][ModList] Found %u mod(s) in %s\n", static_cast<unsigned int>(Mods.size()), MOD_DIR);
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
    if (index < 0 || index >= static_cast<int>(Mods.size()) || index == CurrentMod)
        return;

    CurrentMod = index;
    UpdateSelection();
    DbgPrint("[codxe][IW3][ModList] Selected mod: %s\n", Mods[CurrentMod].name.c_str());
}

void ModList::UpdateSelection()
{
    const mod_metadata::Entry *entry = Mods.empty() ? nullptr : &Mods[CurrentMod];
    SetMetadata("ui_codxe_mod_name", entry ? entry->displayName.c_str() : "");
    const int extraTitleHeight = entry ? MeasureModTitleWrapHeight(entry->displayName.c_str()) : 0;
    SetMetadata("ui_codxe_mod_title_wrap_height", va("%d", extraTitleHeight));
    SetMetadata("ui_codxe_mod_description", entry ? entry->description.c_str() : "");
    SetMetadata("ui_codxe_mod_author", entry ? entry->credits.author.c_str() : "");
    SetMetadata("ui_codxe_mod_revision", entry ? entry->credits.revision.c_str() : "");

    bool uploaded = false;
    if (entry && !entry->previewPath.empty())
    {
        const char *const imageName = "codxe_mod_preview";
        GfxImage *preview = DB_FindXAssetHeader(ASSET_TYPE_IMAGE, imageName).image;
        const image::preview::DeviceFunctions backend = {gfxDevice, R_SyncRenderThread, D3D_BlockUntilIdle, nullptr,
                                                         D3D_Reserve};
        if (preview && preview->name && std::strcmp(preview->name, imageName) == 0)
            uploaded = image::preview::UploadDDS(entry->previewPath, preview->texture.basemap, preview->pixels,
                                                 preview->width, preview->height, backend);
    }
    SetMetadata("ui_codxe_mod_has_preview", uploaded ? "1" : "0");
}

void ModList::LoadModsScript(int /*localClientNum*/, const char ** /*args*/)
{
    ScanMods();
    UpdateSelection();
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
