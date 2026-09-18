#include "pch.h"
#include "ui_feeder.h"

#include "fastfiles.h"

namespace iw3
{
namespace mp
{
namespace
{
const float USERMAPS_FEEDER_ID = 60.0f;
const unsigned int FEEDER_BACKGROUND_COLUMN = 0;
const unsigned int FEEDER_BACKGROUND_END_COLUMN = 1;
const unsigned int FEEDER_NAME_COLUMN = 2;
const unsigned int FEEDER_ACTION_COLUMN = 3;
const char *const FEEDER_ACTION_GLYPH = "\x01";
const char *const FEEDER_BACKGROUND_MATERIAL = "gradient_fadein";
const char *const FEEDER_BACKGROUND_END_MATERIAL = "button_highlight_end";

std::vector<FeederEntry> usermaps;
int selectedUsermap = 0;
bool usermapsScanned = false;
dvar_s *ui_codxe_usermap_counter = NULL;

std::string ReadDisplayName(const std::string &descriptionPath, const std::string &fallback)
{
    std::string displayName = filesystem::ReadFileToString(descriptionPath);
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

bool ContainsUsermap(const std::string &name)
{
    for (size_t i = 0; i < usermaps.size(); ++i)
    {
        if (I_stricmp(usermaps[i].name.c_str(), name.c_str()) == 0)
            return true;
    }

    return false;
}

void UpdateUsermapCounter()
{
    if (!ui_codxe_usermap_counter)
        return;

    char counter[32] = "";
    if (!usermaps.empty())
    {
        _snprintf_s(counter, ARRAYSIZE(counter), _TRUNCATE, "%u / %u", static_cast<unsigned int>(selectedUsermap + 1),
                    static_cast<unsigned int>(usermaps.size()));
    }

    Dvar_SetStringFromSource(ui_codxe_usermap_counter, counter, DVAR_SOURCE_INTERNAL);
}

void ScanUsermaps()
{
    std::string selectedName;
    if (selectedUsermap >= 0 && selectedUsermap < static_cast<int>(usermaps.size()))
        selectedName = usermaps[selectedUsermap].name;

    usermaps.clear();
    selectedUsermap = 0;
    usermapsScanned = true;
    UIFeeder::SetSelectedIndex(USERMAPS_FEEDER_ID, selectedUsermap);

    const char *usermapsDirectory = FastFiles::GetUsermapsDirectory();
    const std::string searchPattern = filesystem::JoinPath(usermapsDirectory, "*");
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(searchPattern.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        UpdateUsermapCounter();
        DbgPrint("[codxe][IW3][UIFeeder] Usermap directory is unavailable: %s\n", usermapsDirectory);
        return;
    }

    do
    {
        const std::string name = findData.cFileName;
        if (name == "." || name == ".." || (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ||
            ContainsUsermap(name))
        {
            continue;
        }

        const std::string fastfile = FastFiles::GetUsermapFastfilePath(name.c_str());
        if (!filesystem::FileExists(fastfile.c_str()))
            continue;

        FeederEntry entry;
        entry.name = name;
        const std::string directory = filesystem::JoinPath(usermapsDirectory, name.c_str());
        entry.displayName = ReadDisplayName(filesystem::JoinPath(directory.c_str(), "description.txt"), name);
        usermaps.push_back(entry);
    } while (FindNextFileA(findHandle, &findData) != 0);

    FindClose(findHandle);
    std::sort(usermaps.begin(), usermaps.end(), [](const FeederEntry &left, const FeederEntry &right)
              { return I_stricmp(left.displayName.c_str(), right.displayName.c_str()) < 0; });

    for (size_t i = 0; i < usermaps.size(); ++i)
    {
        if (I_stricmp(usermaps[i].name.c_str(), selectedName.c_str()) == 0)
        {
            selectedUsermap = static_cast<int>(i);
            break;
        }
    }

    UIFeeder::SetSelectedIndex(USERMAPS_FEEDER_ID, selectedUsermap);
    UpdateUsermapCounter();

    DbgPrint("[codxe][IW3][UIFeeder] Found %u usermap(s) in %s\n", static_cast<unsigned int>(usermaps.size()),
             usermapsDirectory);
}

void EnsureUsermapsScanned()
{
    if (!usermapsScanned)
        ScanUsermaps();
}

int GetUsermapCount()
{
    EnsureUsermapsScanned();
    return static_cast<int>(usermaps.size());
}

const char *GetUsermapText(int index)
{
    EnsureUsermapsScanned();
    return index >= 0 && index < static_cast<int>(usermaps.size()) ? usermaps[index].displayName.c_str() : "";
}

void SelectUsermap(int index)
{
    EnsureUsermapsScanned();
    if (index < 0 || index >= static_cast<int>(usermaps.size()))
        return;

    selectedUsermap = index;
    UpdateUsermapCounter();
    DbgPrint("[codxe][IW3][UIFeeder] Selected usermap: %s\n", usermaps[selectedUsermap].name.c_str());
}

} // namespace

void UIFeeder::LoadUsermapsScript(int /*localClientNum*/, const char ** /*args*/)
{
    ScanUsermaps();
}

void UIFeeder::ApplyMapScript(int /*localClientNum*/, const char ** /*args*/)
{
    EnsureUsermapsScanned();
    if (selectedUsermap < 0 || selectedUsermap >= static_cast<int>(usermaps.size()))
        return;

    dvar_s *ui_mapname = Dvar_FindMalleableVar("ui_mapname");
    if (!ui_mapname)
    {
        DbgPrint("[codxe][IW3][UIFeeder] Could not find ui_mapname\n");
        return;
    }

    const FeederEntry &usermap = usermaps[selectedUsermap];
    Dvar_SetStringFromSource(ui_mapname, usermap.name.c_str(), DVAR_SOURCE_INTERNAL);
    Party_SetDisplayMapName(usermap.name.c_str());

    dvar_s *ui_mapname_text = Dvar_FindMalleableVar("ui_mapname_text");
    if (!ui_mapname_text)
        ui_mapname_text = Dvar_RegisterString("ui_mapname_text", "", DVAR_FLAG_NONE, "The selected map display name");

    if (ui_mapname_text)
        Dvar_SetStringFromSource(ui_mapname_text, usermap.name.c_str(), DVAR_SOURCE_INTERNAL);

    DbgPrint("[codxe][IW3][UIFeeder] Set selected usermap: %s\n", usermap.name.c_str());
}

std::map<float, UIFeederCallbacks> UIFeeder::Feeders;
std::map<float, int> UIFeeder::SelectedIndices;
Detour UIFeeder::UI_FeederCount_Detour;
Detour UIFeeder::UI_FeederItemColor_Detour;
Detour UIFeeder::UI_FeederItemText_Detour;
Detour UIFeeder::UI_FeederSelection_Detour;
Detour UIFeeder::Item_ListBox_Scroll_Detour;

void UIFeeder::Add(float feederID, UIFeederGetItemCount_t getItemCount, UIFeederGetItemText_t getItemText,
                   UIFeederSelect_t select)
{
    if (!getItemCount || !getItemText || !select)
        return;

    UIFeederCallbacks callbacks = {getItemCount, getItemText, select};
    Feeders[feederID] = callbacks;
    SelectedIndices[feederID] = 0;
}

void UIFeeder::SetSelectedIndex(float feederID, int index)
{
    SelectedIndices[feederID] = index;
}

void UIFeeder::OnDvarInit()
{
    ui_codxe_usermap_counter =
        Dvar_RegisterString("ui_codxe_usermap_counter", "", DVAR_FLAG_NONE, "The selected custom map index");
}

int UIFeeder::UI_FeederCount_Hook(int localClientNum, itemDef_s *item, float feederID)
{
    const std::map<float, UIFeederCallbacks>::const_iterator feeder = Feeders.find(feederID);
    if (feeder == Feeders.end())
        return UI_FeederCount_Detour.GetOriginal<UI_FeederCount_t>()(localClientNum, item, feederID);

    const int count = feeder->second.getItemCount();
    if (item && localClientNum >= 0 && localClientNum < 4)
    {
        int index = SelectedIndices[feederID];
        if (index < 0 || index >= count)
        {
            index = 0;
            SetSelectedIndex(feederID, index);
        }

        item->cursorPos[localClientNum] = index;
    }

    return count;
}

const char *UIFeeder::UI_FeederItemText_Hook(int localClientNum, itemDef_s *item, float feederID, int index,
                                             unsigned int column, Material **handle)
{
    const std::map<float, UIFeederCallbacks>::const_iterator feeder = Feeders.find(feederID);
    if (feeder == Feeders.end())
    {
        return UI_FeederItemText_Detour.GetOriginal<UI_FeederItemText_t>()(localClientNum, item, feederID, index,
                                                                           column, handle);
    }

    if (handle)
        *handle = NULL;

    if (column == FEEDER_BACKGROUND_COLUMN || column == FEEDER_BACKGROUND_END_COLUMN)
    {
        const char *materialName =
            column == FEEDER_BACKGROUND_COLUMN ? FEEDER_BACKGROUND_MATERIAL : FEEDER_BACKGROUND_END_MATERIAL;

        if (handle)
            *handle = DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, materialName).material;

        return "";
    }

    if (column == FEEDER_ACTION_COLUMN)
        return SelectedIndices[feederID] == index ? FEEDER_ACTION_GLYPH : "";

    return column == FEEDER_NAME_COLUMN ? feeder->second.getItemText(index) : "";
}

void UIFeeder::UI_FeederItemColor_Hook(int localClientNum, itemDef_s *item, float feederID, int index, int column,
                                       float *color)
{
    if (Feeders.find(feederID) != Feeders.end())
    {
        if (column == FEEDER_BACKGROUND_COLUMN || column == FEEDER_BACKGROUND_END_COLUMN)
        {
            const bool selected = SelectedIndices[feederID] == index;
            color[0] = 0.9f;
            color[1] = selected ? 0.95f : 0.9f;
            color[2] = 1.0f;
            color[3] = selected ? 0.25f : 0.07f;
        }
        else
        {
            color[0] = 1.0f;
            color[1] = 1.0f;
            color[2] = 1.0f;
            color[3] = 0.9f;
        }

        return;
    }

    UI_FeederItemColor_Detour.GetOriginal<UI_FeederItemColor_t>()(localClientNum, item, feederID, index, column, color);
}

void UIFeeder::UI_FeederSelection_Hook(int localClientNum, float feederID, itemDef_s *item, int index)
{
    const std::map<float, UIFeederCallbacks>::const_iterator feeder = Feeders.find(feederID);
    if (feeder == Feeders.end())
    {
        UI_FeederSelection_Detour.GetOriginal<UI_FeederSelection_t>()(localClientNum, feederID, item, index);
        return;
    }

    if (index < 0 || index >= feeder->second.getItemCount())
        return;

    SetSelectedIndex(feederID, index);
    feeder->second.select(index);
}

void UIFeeder::Item_ListBox_Scroll_Hook(int localClientNum, itemDef_s *item, int max, int scrollMax, int viewMax,
                                        int delta)
{
    if (item && Feeders.find(item->special) != Feeders.end() && localClientNum >= 0 && localClientNum < 4 && max > 0)
    {
        if (delta < 0 && item->cursorPos[localClientNum] == 0)
        {
            Item_ListBox_SetCursorPos(localClientNum, item, viewMax, max);
            return;
        }

        if (delta > 0 && item->cursorPos[localClientNum] == max)
        {
            Item_ListBox_SetCursorPos(localClientNum, item, viewMax, 0);
            return;
        }
    }

    Item_ListBox_Scroll_Detour.GetOriginal<Item_ListBox_Scroll_t>()(localClientNum, item, max, scrollMax, viewMax,
                                                                    delta);
}

UIFeeder::UIFeeder()
{
    Add(USERMAPS_FEEDER_ID, GetUsermapCount, GetUsermapText, SelectUsermap);

    UI_FeederCount_Detour = Detour(UI_FeederCount, UI_FeederCount_Hook);
    UI_FeederCount_Detour.Install();

    UI_FeederItemColor_Detour = Detour(UI_FeederItemColor, UI_FeederItemColor_Hook);
    UI_FeederItemColor_Detour.Install();

    UI_FeederItemText_Detour = Detour(UI_FeederItemText, UI_FeederItemText_Hook);
    UI_FeederItemText_Detour.Install();

    UI_FeederSelection_Detour = Detour(UI_FeederSelection, UI_FeederSelection_Hook);
    UI_FeederSelection_Detour.Install();

    Item_ListBox_Scroll_Detour = Detour(Item_ListBox_Scroll, Item_ListBox_Scroll_Hook);
    Item_ListBox_Scroll_Detour.Install();
}

UIFeeder::~UIFeeder()
{
    Item_ListBox_Scroll_Detour.Remove();
    UI_FeederSelection_Detour.Remove();
    UI_FeederItemText_Detour.Remove();
    UI_FeederItemColor_Detour.Remove();
    UI_FeederCount_Detour.Remove();
    SelectedIndices.clear();
    Feeders.clear();
}
} // namespace mp
} // namespace iw3
