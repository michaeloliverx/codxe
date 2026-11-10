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
std::vector<usermaps::Entry> maps;
std::string previewMap;
bool previewValid = false;
int selected[4] = {};
dvar_t *ui_codxe_usermap_count = nullptr;
dvar_t *ui_codxe_usermap_name = nullptr;
dvar_t *ui_codxe_usermap_description = nullptr;
} // namespace

void Usermaps::OnDvarInit()
{
    ui_codxe_usermap_count = Dvar_RegisterString("ui_codxe_usermap_count", "0", 0, "Number of installed custom maps");
    ui_codxe_usermap_name = Dvar_RegisterString("ui_codxe_usermap_name", "", 0, "Selected custom map name");
    ui_codxe_usermap_description =
        Dvar_RegisterString("ui_codxe_usermap_description", "", 0, "Selected custom map description");
    Dvar_RegisterString("ui_codxe_usermap_has_preview", "0", 0, "Selected map has a preview");
}

int Usermaps::Count()
{
    return static_cast<int>(maps.size());
}

const char *Usermaps::Text(int localClientNum, int index, int column)
{
    if (index < 0 || index >= Count())
        return "";
    if (column == 0)
        return maps[index].displayName.c_str();
    if (column == 1 && localClientNum >= 0 && localClientNum < static_cast<int>(ARRAYSIZE(selected)) &&
        index == selected[localClientNum])
        return "\x01"; // Stock A-button text glyph, as used by PLATFORM_UI_SELECTBUTTON.
    return "";
}

void Usermaps::Select(int localClientNum, int index)
{
    if (localClientNum >= 0 && localClientNum < static_cast<int>(ARRAYSIZE(selected)) && index >= 0 && index < Count())
    {
        selected[localClientNum] = index;
        if (ui_codxe_usermap_name)
            Dvar_SetString(ui_codxe_usermap_name, maps[index].displayName.c_str());
        if (ui_codxe_usermap_description)
            Dvar_SetString(ui_codxe_usermap_description, maps[index].description.c_str());
        Dvar_SetStringByName("ui_codxe_usermap_has_preview", EnsurePreview(maps[index].name.c_str()) ? "1" : "0");
    }
}

void Usermaps::Load(int localClientNum, const char ** /*args*/)
{
    if (localClientNum < 0 || localClientNum >= static_cast<int>(ARRAYSIZE(selected)))
        return;
    maps.clear();
    Arenas::Reload();
    memset(selected, 0, sizeof(selected));
    if (ui_codxe_usermap_name)
        Dvar_SetString(ui_codxe_usermap_name, "");
    if (ui_codxe_usermap_description)
        Dvar_SetString(ui_codxe_usermap_description, "");
    maps = usermaps::ScanMultiplayer();
    for (size_t i = 0; i < maps.size(); ++i)
    {
        maps[i].displayName = Arenas::DisplayName(maps[i].name.c_str());
        maps[i].description = Arenas::Description(maps[i].name.c_str());
    }
    if (ui_codxe_usermap_count)
        Dvar_SetString(ui_codxe_usermap_count, va("%d", Count()));
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
    if (index < 0 || index >= Count())
        return;
    const char *name = maps[index].name.c_str();
    Dvar_SetStringByName("ui_mapname", name);
    Dvar_SetStringByName("ui_mapname_text", maps[index].displayName.c_str());
    Party_SetDisplayMapName(name);
    DbgPrint("[codxe][IW4 TU6][Usermaps] Applied %s\n", name);
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
    UIScript::Add("LoadUsermaps", Load);
    UIScript::Add("ApplyMap", Apply);
    UIScript::Add("CloseUsermapPreview", Close);
}
} // namespace mp_tu6
} // namespace iw4
