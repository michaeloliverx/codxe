#include "pch.h"
#include "usermaps.h"
#include "ui_feeder.h"

#include "common/usermaps.h"
#include "image/usermap_preview.h"

namespace iw3
{
namespace mp
{
namespace
{
const float USERMAPS_FEEDER_ID = 60.0f;
const float TAGS_FEEDER_ID = 66.0f;
std::vector<usermaps::Entry> maps;
usermaps::TagFilter filter;
int highlightedTag = 0;
int selectedUsermap = 0;
bool usermapsScanned = false;
std::string previewMap;
Detour StringTable_Lookup_Detour;
Detour UI_GetMapDisplayName_Detour;

const char *UI_GetMapDisplayName_Hook(const char *mapName)
{
    // The connection/loading screen uses this helper rather than mapsTable.
    if (mapName)
    {
        for (size_t i = 0; i < maps.size(); ++i)
        {
            if (I_stricmp(maps[i].name.c_str(), mapName) == 0)
                return maps[i].displayName.c_str();
        }
    }
    return UI_GetMapDisplayName_Detour.GetOriginal<decltype(UI_GetMapDisplayName)>()(mapName);
}

void SetMetadata(const char *name, const char *value)
{
    dvar_s *dvar = Dvar_FindMalleableVar(name);
    if (dvar)
        Dvar_SetStringFromSource(dvar, value, DVAR_SOURCE_INTERNAL);
}

std::string FormatMapTags(const std::vector<std::string> &tags)
{
    std::string text;
    for (size_t i = 0; i < tags.size(); ++i)
    {
        if (!text.empty())
            text += ", ";
        text += tags[i];
    }
    if (text.empty())
        return text;

    // Match the font, scale, and width of usermap_tags in the menu.
    const float scale = 0.375f;
    const unsigned int width = 207;
    Font_s *font = UI_GetFontHandle(1, scale);
    if (!font || font->pixelHeight <= 0)
        return text;

    const float drawScale = scale * 48.0f / font->pixelHeight;
    if (R_TextWidth(text.c_str(), 0, font) * drawScale <= width)
        return text;

    const char ellipsis[] = "...";
    const float prefixWidth = width - R_TextWidth(ellipsis, 0, font) * drawScale;
    if (prefixWidth <= 0)
        return ellipsis;

    const char *start = text.c_str();
    const char *end = start;
    int chars = 0;
    while (*end && R_TextWidth(start, ++chars, font) * drawScale <= prefixWidth)
    {
        int bytes = 0;
        SEH_DecodeLetter(end[0], end[1], &bytes);
        if (bytes <= 0)
            break;
        end += bytes;
    }
    while (end > start && (end[-1] == ' ' || end[-1] == ','))
        --end;
    return std::string(start, end) + ellipsis;
}

void UpdateUsermapSelection()
{
    const usermaps::Entry *entry =
        selectedUsermap >= 0 && selectedUsermap < static_cast<int>(maps.size()) ? &maps[selectedUsermap] : nullptr;
    char counter[32] = "";
    if (entry)
        _snprintf_s(counter, ARRAYSIZE(counter), _TRUNCATE, "%d / %d", filter.RowForMap(selectedUsermap) + 1,
                    filter.MapCount());
    SetMetadata("ui_codxe_usermap_counter", counter);
    SetMetadata("ui_codxe_usermap_name", entry ? entry->displayName.c_str() : "");
    SetMetadata("ui_codxe_usermap_description", entry ? entry->description.c_str() : "");
    SetMetadata("ui_codxe_usermap_author", entry ? entry->credits.author.c_str() : "");
    SetMetadata("ui_codxe_usermap_revision", entry ? entry->credits.revision.c_str() : "");
    SetMetadata("ui_codxe_usermap_tags", entry ? FormatMapTags(entry->tags).c_str() : "");
    char count[16];
    _snprintf_s(count, ARRAYSIZE(count), _TRUNCATE, "%d", filter.MapCount());
    SetMetadata("ui_codxe_usermap_count", count);

    // Reuse one zone-owned texture rather than allocating one image per map.
    bool uploaded = false;
    if (entry && !entry->previewPath.empty())
    {
        GfxImage *preview = DB_FindXAssetHeader(ASSET_TYPE_IMAGE, "codxe_usermap_preview").image;
        const image::preview::DeviceFunctions backend = {gfxDevice, R_SyncRenderThread, D3D_BlockUntilIdle, nullptr,
                                                         D3D_Reserve};
        if (preview && preview->name && std::strcmp(preview->name, "codxe_usermap_preview") == 0)
            uploaded = image::preview::UploadDDS(entry->previewPath, preview->texture.basemap, preview->pixels,
                                                 preview->width, preview->height, backend);
    }
    previewMap = uploaded && entry ? entry->name : "";
    SetMetadata("ui_codxe_usermap_has_preview", uploaded ? "1" : "0");
}

void UpdateFilterLabel()
{
    SetMetadata("ui_codxe_usermap_has_tags", filter.HasTags() ? "1" : "0");
    SetMetadata("ui_codxe_usermap_tag", filter.Prompt().c_str());
}

void ScanUsermaps()
{
    const std::string previous =
        selectedUsermap >= 0 && selectedUsermap < static_cast<int>(maps.size()) ? maps[selectedUsermap].name : "";
    maps = usermaps::ScanMultiplayer();
    filter.Reset(maps);
    UpdateFilterLabel();
    selectedUsermap = 0;
    usermapsScanned = true;
    for (size_t i = 0; i < maps.size(); ++i)
    {
        if (I_stricmp(maps[i].name.c_str(), previous.c_str()) == 0)
            selectedUsermap = static_cast<int>(i);
    }
    UIFeeder::SetSelectedIndex(USERMAPS_FEEDER_ID, selectedUsermap);
    UpdateUsermapSelection();
    DbgPrint("[codxe][IW3][UIFeeder] Found %u MP map(s)\n", static_cast<unsigned int>(maps.size()));
}

void EnsureUsermapsScanned()
{
    if (!usermapsScanned)
        ScanUsermaps();
}

int GetUsermapCount()
{
    EnsureUsermapsScanned();
    return filter.MapCount();
}

const char *GetUsermapText(int index)
{
    EnsureUsermapsScanned();
    const int map = filter.MapIndex(index);
    return map >= 0 ? maps[map].displayName.c_str() : "";
}

void SelectMap(int index)
{
    EnsureUsermapsScanned();
    if (index < 0 || index >= static_cast<int>(maps.size()))
        return;

    if (selectedUsermap == index)
        return;
    selectedUsermap = index;
    UpdateUsermapSelection();
    DbgPrint("[codxe][IW3][UIFeeder] Selected usermap: %s\n", maps[selectedUsermap].name.c_str());
}

void SelectUsermap(int row)
{
    SelectMap(filter.MapIndex(row));
}

int GetTagCount()
{
    return filter.TagCount();
}

const char *GetTagText(int row)
{
    return filter.TagText(row);
}

void HighlightTag(int row)
{
    highlightedTag = row;
}

// The stock lobby uses mapsTable column 4. Supply its material through the
// same lookup, so every stock lobby can display usermaps without menu copies.
const char *StringTable_Lookup_Hook(const StringTable *table, int comparisonColumn, const char *value, int valueColumn)
{
    if (table && table->name && value && comparisonColumn == 0 && valueColumn == 4 &&
        I_stricmp(table->name, "mp/mapsTable.csv") == 0)
    {
        for (size_t i = 0; i < maps.size(); ++i)
        {
            if (I_stricmp(maps[i].name.c_str(), value) == 0)
                return I_stricmp(previewMap.c_str(), value) == 0 ? "$codxe_usermap_preview" : "compass_map_default";
        }
    }
    return StringTable_Lookup_Detour.GetOriginal<decltype(StringTable_Lookup)>()(table, comparisonColumn, value,
                                                                                 valueColumn);
}

} // namespace

void Usermaps::LoadScript(int /*localClientNum*/, const char ** /*args*/)
{
    ScanUsermaps();
}

void Usermaps::LoadFilters(int, const char **)
{
    highlightedTag = filter.SelectedTag();
    UIFeeder::SetSelectedIndex(TAGS_FEEDER_ID, highlightedTag);
}

void Usermaps::ApplyFilter(int, const char **)
{
    filter.SelectTag(highlightedTag);
    const int row = (std::max)(0, filter.RowForMap(selectedUsermap));
    selectedUsermap = filter.MapIndex(row);
    UIFeeder::SetSelectedIndex(USERMAPS_FEEDER_ID, row);
    UpdateFilterLabel();
    UpdateUsermapSelection();
}

void Usermaps::ApplyScript(int /*localClientNum*/, const char ** /*args*/)
{
    EnsureUsermapsScanned();
    if (selectedUsermap < 0 || selectedUsermap >= static_cast<int>(maps.size()))
        return;

    dvar_s *ui_mapname = Dvar_FindMalleableVar("ui_mapname");
    if (!ui_mapname)
    {
        DbgPrint("[codxe][IW3][UIFeeder] Could not find ui_mapname\n");
        return;
    }

    const usermaps::Entry &usermap = maps[selectedUsermap];
    Dvar_SetStringFromSource(ui_mapname, usermap.name.c_str(), DVAR_SOURCE_INTERNAL);
    Party_SetDisplayMapName(usermap.name.c_str());

    dvar_s *ui_mapname_text = Dvar_FindMalleableVar("ui_mapname_text");
    if (!ui_mapname_text)
        ui_mapname_text = Dvar_RegisterString("ui_mapname_text", "", DVAR_FLAG_NONE, "The selected map display name");

    if (ui_mapname_text)
        Dvar_SetStringFromSource(ui_mapname_text, usermap.displayName.c_str(), DVAR_SOURCE_INTERNAL);

    DbgPrint("[codxe][IW3][UIFeeder] Set selected usermap: %s\n", usermap.name.c_str());
}

void Usermaps::CloseScript(int /*localClientNum*/, const char ** /*args*/)
{
    // One texture serves the browser and lobby. Restore the chosen map after
    // browsing another entry and cancelling, rather than retaining that image.
    const char *current = Dvar_GetString("ui_mapname");
    for (size_t i = 0; i < maps.size(); ++i)
    {
        if (I_stricmp(maps[i].name.c_str(), current) == 0)
        {
            SelectMap(static_cast<int>(i));
            UIFeeder::SetSelectedIndex(USERMAPS_FEEDER_ID, filter.RowForMap(selectedUsermap));
            break;
        }
    }
}

void Usermaps::OnDvarInit()
{
    const char *names[] = {"ui_codxe_usermap_counter", "ui_codxe_usermap_name", "ui_codxe_usermap_description",
                           "ui_codxe_usermap_author", "ui_codxe_usermap_revision", "ui_codxe_usermap_tags",
                           "ui_codxe_usermap_count", "ui_codxe_usermap_has_preview", "ui_codxe_usermap_has_tags",
                           "ui_codxe_usermap_tag"};
    for (size_t i = 0; i < ARRAYSIZE(names); ++i)
        Dvar_RegisterString(names[i], "", DVAR_FLAG_NONE, "Custom map metadata");
}

Usermaps::Usermaps()
{
    UIFeeder::Add(USERMAPS_FEEDER_ID, GetUsermapCount, GetUsermapText, SelectUsermap);
    UIFeeder::Add(TAGS_FEEDER_ID, GetTagCount, GetTagText, HighlightTag);
    StringTable_Lookup_Detour = Detour(StringTable_Lookup, StringTable_Lookup_Hook);
    StringTable_Lookup_Detour.Install();
    UI_GetMapDisplayName_Detour = Detour(UI_GetMapDisplayName, UI_GetMapDisplayName_Hook);
    UI_GetMapDisplayName_Detour.Install();
}
} // namespace mp
} // namespace iw3
