#include "pch.h"
#include "usermaps.h"

#include "arenas.h"
#include "common/usermaps.h"
#include "ui_feeder.h"
#include "ui_script.h"
#include "usermap_preview.h"

namespace iw4
{
namespace mp_tu6
{
namespace
{
// Matches FEEDER_CODXE_USERMAPS in zones/iw4/raw/ui_mp/menudef.h.
const int USERMAPS_FEEDER = 60;
const int TAGS_FEEDER = 66;
std::vector<usermaps::Entry> maps;
usermaps::TagFilter filter;
int highlightedTag = 0;
std::string previewMap;
bool previewValid = false;
int selected[4] = {};
dvar_t *ui_codxe_usermap_count = nullptr;
dvar_t *ui_codxe_usermap_name = nullptr;
dvar_t *ui_codxe_usermap_description = nullptr;
dvar_t *ui_codxe_usermap_author = nullptr;
dvar_t *ui_codxe_usermap_revision = nullptr;

void SetMapMetadata(const usermaps::Entry *entry)
{
    if (ui_codxe_usermap_name)
        Dvar_SetString(ui_codxe_usermap_name, entry ? entry->displayName.c_str() : "");
    if (ui_codxe_usermap_description)
        Dvar_SetString(ui_codxe_usermap_description, entry ? entry->description.c_str() : "");
    if (ui_codxe_usermap_author)
        Dvar_SetString(ui_codxe_usermap_author, entry ? entry->credits.author.c_str() : "");
    if (ui_codxe_usermap_revision)
        Dvar_SetString(ui_codxe_usermap_revision, entry ? entry->credits.revision.c_str() : "");
}

void UpdateFilterLabel()
{
    if (ui_codxe_usermap_count)
        Dvar_SetString(ui_codxe_usermap_count, va("%d", filter.MapCount()));
    Dvar_SetStringByName("ui_codxe_usermap_has_tags", filter.HasTags() ? "1" : "0");
    Dvar_SetStringByName("ui_codxe_usermap_tag", filter.Prompt().c_str());
}

void LoadFilters(int localClientNum, const char **)
{
    highlightedTag = filter.SelectedTag();
    Menu_SetFeederSelection(UI_GetClientDC(localClientNum), nullptr, TAGS_FEEDER, highlightedTag, "codxe_usermap_filter");
}

int GetTagCount()
{
    return filter.TagCount();
}

const char *GetTagText(int, int row, int column)
{
    if (column == 0)
        return filter.TagText(row);
    return column == 1 && row == highlightedTag ? "\x01" : "";
}

void HighlightTag(int, int row)
{
    highlightedTag = row;
}
} // namespace

void Usermaps::OnDvarInit()
{
    ui_codxe_usermap_count = Dvar_RegisterString("ui_codxe_usermap_count", "0", 0, "Number of installed custom maps");
    ui_codxe_usermap_name = Dvar_RegisterString("ui_codxe_usermap_name", "", 0, "Selected custom map name");
    ui_codxe_usermap_description =
        Dvar_RegisterString("ui_codxe_usermap_description", "", 0, "Selected custom map description");
    ui_codxe_usermap_author = Dvar_RegisterString("ui_codxe_usermap_author", "", 0, "Selected custom map author");
    ui_codxe_usermap_revision = Dvar_RegisterString("ui_codxe_usermap_revision", "", 0, "Selected custom map revision");
    Dvar_RegisterString("ui_codxe_usermap_has_preview", "0", 0, "Selected map has a preview");
    Dvar_RegisterString("ui_codxe_usermap_has_tags", "0", 0, "Custom maps have filter tags");
    Dvar_RegisterString("ui_codxe_usermap_tag", "", 0, "Selected custom map filter");
}

int Usermaps::Count()
{
    return filter.MapCount();
}

const char *Usermaps::Text(int localClientNum, int index, int column)
{
    const int map = filter.MapIndex(index);
    if (map < 0)
        return "";
    if (column == 0)
        return maps[map].displayName.c_str();
    if (column == 1 && localClientNum >= 0 && localClientNum < static_cast<int>(ARRAYSIZE(selected)) &&
        map == selected[localClientNum])
        return "\x01"; // Stock A-button text glyph, as used by PLATFORM_UI_SELECTBUTTON.
    return "";
}

void Usermaps::Select(int localClientNum, int index)
{
    const int map = filter.MapIndex(index);
    if (localClientNum >= 0 && localClientNum < static_cast<int>(ARRAYSIZE(selected)) && map >= 0)
    {
        selected[localClientNum] = map;
        SetMapMetadata(&maps[map]);
        Dvar_SetStringByName("ui_codxe_usermap_has_preview", EnsurePreview(maps[map].name.c_str()) ? "1" : "0");
    }
}

void Usermaps::Load(int localClientNum, const char ** /*args*/)
{
    if (localClientNum < 0 || localClientNum >= static_cast<int>(ARRAYSIZE(selected)))
        return;
    maps.clear();
    Arenas::Reload();
    memset(selected, 0, sizeof(selected));
    SetMapMetadata(nullptr);
    Dvar_SetStringByName("ui_codxe_usermap_has_preview", "0");
    maps = usermaps::ScanMultiplayer();
    for (size_t i = 0; i < maps.size(); ++i)
    {
        maps[i].displayName = Arenas::DisplayName(maps[i].name.c_str());
        maps[i].description = Arenas::Description(maps[i].name.c_str());
    }
    filter.Reset(maps);
    UpdateFilterLabel();
    Menu_SetFeederSelection(UI_GetClientDC(localClientNum), nullptr, USERMAPS_FEEDER, 0, "settings_usermaps");
    if (!maps.empty())
        Select(localClientNum, 0);
    DbgPrint("[codxe][IW4 TU6][Usermaps] Found %u custom map(s)\n", static_cast<unsigned int>(maps.size()));
}

void Usermaps::Apply(int localClientNum, const char ** /*args*/)
{
    if (localClientNum < 0 || localClientNum >= static_cast<int>(ARRAYSIZE(selected)))
        return;
    const int index = selected[localClientNum];
    if (filter.RowForMap(index) < 0)
        return;
    const char *name = maps[index].name.c_str();
    Dvar_SetStringByName("ui_mapname", name);
    Dvar_SetStringByName("ui_mapname_text", maps[index].displayName.c_str());
    Party_SetDisplayMapName(name);
    DbgPrint("[codxe][IW4 TU6][Usermaps] Applied %s\n", name);
}

void Usermaps::ApplyFilter(int localClientNum, const char **)
{
    if (localClientNum < 0 || localClientNum >= static_cast<int>(ARRAYSIZE(selected)))
        return;
    filter.SelectTag(highlightedTag);
    const int row = (std::max)(0, filter.RowForMap(selected[localClientNum]));
    UpdateFilterLabel();
    Menu_SetFeederSelection(UI_GetClientDC(localClientNum), nullptr, USERMAPS_FEEDER, row, "settings_usermaps");
    Select(localClientNum, row);
}

bool Usermaps::EnsurePreview(const char *map)
{
    if (!usermaps::IsMultiplayerName(map))
        return false;
    if (previewMap == map)
        return previewValid;
    previewMap = map;
    previewValid = false;
    const std::string fastfile = usermaps::FastfilePath(map);
    if (fastfile.empty() || !filesystem::FileExists(fastfile.c_str()))
        return false;
    const std::string directory = filesystem::JoinPath(usermaps::GetDirectory(), map);
    const std::string path = filesystem::JoinPath(directory.c_str(), "preview.dds");
    previewValid = filesystem::FileExists(path.c_str()) && UsermapPreview::Load(path);
    return previewValid;
}

void Usermaps::Close(int, const char **)
{
    EnsurePreview(GetDvarString("ui_mapname"));
}

Usermaps::Usermaps()
{
    const UIFeeder::Callbacks callbacks = {Count, Text, Select};
    UIFeeder::Add(static_cast<float>(USERMAPS_FEEDER), callbacks);
    const UIFeeder::Callbacks tags = {GetTagCount, GetTagText, HighlightTag};
    UIFeeder::Add(static_cast<float>(TAGS_FEEDER), tags);
    UIScript::Add("LoadUsermaps", Load);
    UIScript::Add("ApplyMap", Apply);
    UIScript::Add("CloseUsermapPreview", Close);
    UIScript::Add("LoadUsermapFilters", LoadFilters);
    UIScript::Add("ApplyUsermapFilter", ApplyFilter);
}
} // namespace mp_tu6
} // namespace iw4
