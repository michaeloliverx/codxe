#include "pch.h"
#include "usermaps.h"

#include "common/usermaps.h"
#include "image/usermap_preview.h"

namespace t4
{
namespace mp
{
namespace
{
std::vector<usermaps::Entry> maps;
int selected = 0;
Detour StringTable_Lookup_Detour;
Detour UI_GetMapDisplayName_Detour;
Detour UI_GetMapLoadName_Detour;

const char *FindMapTitle(const char *mapName)
{
    if (mapName)
    {
        for (size_t i = 0; i < maps.size(); ++i)
        {
            if (_stricmp(maps[i].name.c_str(), mapName) == 0)
                return maps[i].displayName.c_str();
        }
    }
    return nullptr;
}

const char *UI_GetMapDisplayName_Hook(const char *mapName)
{
    const char *title = FindMapTitle(mapName);
    return title ? title : UI_GetMapDisplayName_Detour.GetOriginal<decltype(UI_GetMapDisplayName)>()(mapName);
}

const char *UI_GetMapLoadName_Hook(const char *mapName)
{
    // Stock maps have a separate loadname; custom maps use their arena longname.
    const char *title = FindMapTitle(mapName);
    return title ? title : UI_GetMapLoadName_Detour.GetOriginal<decltype(UI_GetMapLoadName)>()(mapName);
}

const char *StringTable_Lookup_Hook(const StringTable *table, int comparisonColumn, const char *value, int valueColumn)
{
    // The lobby caption reads mapsTable's name column directly, bypassing ui_mapname_text.
    // Keep the internal map identifier intact and supply its cached arena title here.
    if (table && table->name && value && comparisonColumn == 0 && valueColumn == 3 &&
        _stricmp(table->name, "mp/mapsTable.csv") == 0)
    {
        const char *title = FindMapTitle(value);
        if (title)
            return title;
    }
    return StringTable_Lookup_Detour.GetOriginal<decltype(StringTable_Lookup)>()(table, comparisonColumn, value,
                                                                                 valueColumn);
}

void UpdateSelection()
{
    const usermaps::Entry *entry =
        selected >= 0 && selected < static_cast<int>(maps.size()) ? &maps[selected] : nullptr;
    Dvar_SetStringByName("ui_codxe_usermap_name", entry ? entry->displayName.c_str() : "");
    Dvar_SetStringByName("ui_codxe_usermap_description", entry ? entry->description.c_str() : "");
    Dvar_SetStringByName("ui_codxe_usermap_mapname", entry ? entry->name.c_str() : "");
    Dvar_SetIntByName("ui_codxe_usermap_count", static_cast<int>(maps.size()));

    // Reuse one zone-owned texture. No previews are retained for unselected maps.
    bool uploaded = false;
    if (entry && !entry->previewPath.empty())
    {
        auto image =
            reinterpret_cast<GfxImage *>(DB_FindXAssetHeader(ASSET_TYPE_IMAGE, "codxe_usermap_preview", false, 0));
        const image::preview::DeviceFunctions backend = {gfxDevice, R_SyncRenderThread, D3D_BlockUntilIdle,
                                                         D3D_InvalidateCaches, nullptr};
        if (image && image->name && std::strcmp(image->name, "codxe_usermap_preview") == 0)
            uploaded = image::preview::UploadDDS(entry->previewPath, image->texture.basemap, image->pixels,
                                                 image->width, image->height, backend);
    }
    Dvar_SetIntByName("ui_codxe_usermap_has_preview", uploaded ? 1 : 0);
}

void Scan(int localClientNum)
{
    const std::string previous = selected >= 0 && selected < static_cast<int>(maps.size()) ? maps[selected].name : "";
    maps = usermaps::ScanMultiplayer();
    selected = 0;
    for (size_t i = 0; i < maps.size(); ++i)
    {
        if (_stricmp(maps[i].name.c_str(), previous.c_str()) == 0)
            selected = static_cast<int>(i);
    }
    Menu_SetFeederSelection(UI_GetContext(localClientNum), nullptr, Usermaps::FEEDER_ID, selected, "settings");
    UpdateSelection();
    DbgPrint("[codxe][T4][Usermaps] Found %u MP map(s)\n", static_cast<unsigned int>(maps.size()));
}
} // namespace

void Usermaps::OnDvarInit()
{
    const char *strings[] = {"ui_codxe_usermap_name", "ui_codxe_usermap_description", "ui_codxe_usermap_mapname"};
    for (size_t i = 0; i < ARRAYSIZE(strings); ++i)
        Dvar_RegisterString(strings[i], "", DVAR_FLAG_NONE, "Custom map metadata");
    Dvar_SetIntByName("ui_codxe_usermap_count", 0);
    Dvar_SetIntByName("ui_codxe_usermap_has_preview", 0);
    Dvar_SetIntByName("settings_usermaps_selected", 0);
}

int Usermaps::Count()
{
    return static_cast<int>(maps.size());
}

const char *Usermaps::Text(int index)
{
    return index >= 0 && index < Count() ? maps[index].displayName.c_str() : "";
}

void Usermaps::Select(int index)
{
    if (index >= 0 && index < Count() && selected != index)
    {
        selected = index;
        UpdateSelection();
    }
}

bool Usermaps::RunMenuScript(int localClientNum, const char *command)
{
    if (_stricmp(command, "LoadUsermaps") == 0)
        Scan(localClientNum);
    else if (_stricmp(command, "CloseUsermaps") == 0)
    {
        // Cancelling the browser restores the lobby's chosen map in the shared texture.
        const char *current = Dvar_GetString("ui_mapname");
        int active = -1;
        for (size_t i = 0; i < maps.size(); ++i)
        {
            if (_stricmp(maps[i].name.c_str(), current) == 0)
                active = static_cast<int>(i);
        }
        if (selected != active)
        {
            selected = active;
            UpdateSelection();
        }
    }
    else if (_stricmp(command, "ApplyUsermap") == 0)
    {
        if (selected >= 0 && selected < Count())
        {
            const usermaps::Entry &entry = maps[selected];
            Dvar_SetStringByName("ui_mapname", entry.name.c_str());
            Dvar_SetStringByName("ui_mapname_text", entry.displayName.c_str());
            Dvar_SetStringByName("ui_preview", entry.name.c_str());
            Cbuf_AddText(localClientNum, "xupdatepartystate\n");
        }
    }
    else
        return false;
    return true;
}

Usermaps::Usermaps()
{
    StringTable_Lookup_Detour = Detour(StringTable_Lookup, StringTable_Lookup_Hook);
    StringTable_Lookup_Detour.Install();
    UI_GetMapDisplayName_Detour = Detour(UI_GetMapDisplayName, UI_GetMapDisplayName_Hook);
    UI_GetMapDisplayName_Detour.Install();
    UI_GetMapLoadName_Detour = Detour(UI_GetMapLoadName, UI_GetMapLoadName_Hook);
    UI_GetMapLoadName_Detour.Install();
}

Usermaps::~Usermaps()
{
    UI_GetMapLoadName_Detour.Remove();
    UI_GetMapDisplayName_Detour.Remove();
    StringTable_Lookup_Detour.Remove();
    maps.clear();
    selected = 0;
}
} // namespace mp
} // namespace t4
