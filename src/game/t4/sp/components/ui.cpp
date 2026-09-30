#include "pch.h"
#include "ui.h"
#include "console.h"

#include "common/endian.h"
#include "image/xenos_texture.h"

#include <cstdlib>
#include <cstdio>

namespace t4
{
namespace sp
{
namespace
{
const float CODXE_USERMAPS_FEEDER = 65.0f;
const char *const CODXE_USERMAPS_DIRECTORY = "usermaps";
const char *const CODXE_PREVIEW_MATERIAL = "$codxe_usermap_preview";
const char *const CODXE_PREVIEW_IMAGE = "codxe_usermap_preview";

const uint32_t DDS_MAGIC = MAKEFOURCC('D', 'D', 'S', ' ');
const uint32_t DDS_HEADER_SIZE = 124;
const uint32_t DDS_PIXEL_FORMAT_SIZE = 32;
const uint32_t DDS_FOURCC = 0x4;
const uint32_t DXT1_FOURCC = MAKEFOURCC('D', 'X', 'T', '1');

struct DDSHeader
{
    uint32_t magic;
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t pitchOrLinearSize;
    uint32_t depth;
    uint32_t mipMapCount;
    uint32_t reserved1[11];
    struct
    {
        uint32_t size;
        uint32_t flags;
        uint32_t fourCC;
        uint32_t rgbBitCount;
        uint32_t rBitMask;
        uint32_t gBitMask;
        uint32_t bBitMask;
        uint32_t aBitMask;
    } pixelFormat;
    uint32_t caps;
    uint32_t caps2;
    uint32_t caps3;
    uint32_t caps4;
    uint32_t reserved2;
};
static_assert(sizeof(DDSHeader) == 128, "");

struct UsermapEntry
{
    std::string name;
    std::string displayName;
    std::string description;
    std::string previewPath;
};

std::vector<UsermapEntry> usermaps;
int selectedUsermap = 0;
bool usermapsScanned = false;
bool usermapDvarsRegistered = false;
bool usermapNameLookupLogged = false;
bool usermapPreviewLookupLogged = false;
GfxImage *previewImage = nullptr;
std::vector<unsigned char> defaultPreviewPixels;

Detour UI_FeederCount_Detour;
Detour UI_FeederItemText_Detour;
Detour UI_FeederSelection_Detour;
Detour StringTable_Lookup_Detour;

void EnsureUsermapDvarsRegistered()
{
    if (usermapDvarsRegistered)
        return;

    Dvar_RegisterBool("ui_codxe_usermap_activate", false, DVAR_FLAG_NONE, "Activate the selected custom map");
    Dvar_RegisterBool("ui_codxe_usermap_active", false, DVAR_FLAG_NONE, "A custom map is selected");
    Dvar_RegisterBool("ui_codxe_usermap_has_preview", false, DVAR_FLAG_NONE,
                      "The selected custom map has a preview image");
    Dvar_RegisterString("ui_codxe_usermap_mode", "", DVAR_FLAG_NONE, "Custom map launch mode");
    Dvar_RegisterString("ui_codxe_usermap_name", "", DVAR_FLAG_NONE, "Selected custom map display name");
    Dvar_RegisterString("ui_codxe_usermap_description", "", DVAR_FLAG_NONE,
                        "Selected custom map description");
    Dvar_RegisterString("ui_codxe_usermap_count", "", DVAR_FLAG_NONE, "Selected custom map index");
    Dvar_RegisterString("ui_codxe_usermap_preview_hint", "", DVAR_FLAG_NONE, "Custom map preview instructions");
    Dvar_RegisterString("ui_codxe_usermap_mapname", "", DVAR_FLAG_NONE, "Selected custom map fastfile name");

    usermapDvarsRegistered = true;
}

bool StartsWithIgnoreCase(const std::string &value, const char *prefix)
{
    const size_t prefixLength = std::strlen(prefix);
    return value.length() >= prefixLength && _strnicmp(value.c_str(), prefix, prefixLength) == 0;
}

void ReadMetadata(const std::string &path, std::string &displayName, std::string &description)
{
    const std::string contents = filesystem::ReadFileToString(path);
    if (contents.empty())
        return;

    HJSONREADER reader = XJSONCreateReader();
    if (!reader || FAILED(XJSONSetBuffer(reader, contents.data(), static_cast<DWORD>(contents.size()), TRUE)))
    {
        if (reader)
            XJSONCloseReader(reader);
        DbgPrint("[codxe][T4 SP][UI] Failed to parse usermap metadata: %s\n", path.c_str());
        return;
    }

    int depth = 0;
    int version = 0;
    JSONTOKENTYPE tokenType;
    DWORD tokenLength;
    DWORD parsed;
    HRESULT result;
    char propertyName[64];
    char value[1024];

    while ((result = XJSONReadToken(reader, &tokenType, &tokenLength, &parsed)) == S_OK)
    {
        if (tokenType == Json_BeginObject || tokenType == Json_BeginArray)
        {
            ++depth;
            continue;
        }
        if (tokenType == Json_EndObject || tokenType == Json_EndArray)
        {
            --depth;
            continue;
        }
        if (tokenType != Json_FieldName)
            continue;

        if (FAILED(XJSONGetTokenValue(reader, propertyName, ARRAYSIZE(propertyName))) ||
            XJSONReadToken(reader, &tokenType, &tokenLength, &parsed) != S_OK)
        {
            result = E_FAIL;
            break;
        }

        if (tokenType == Json_BeginObject || tokenType == Json_BeginArray)
        {
            ++depth;
            continue;
        }
        if (depth != 1 || FAILED(XJSONGetTokenValue(reader, value, ARRAYSIZE(value))))
            continue;

        if (_stricmp(propertyName, "version") == 0 && tokenType == Json_Number)
            version = std::strtol(value, nullptr, 10);
        else if (_stricmp(propertyName, "name") == 0 && tokenType == Json_String)
            displayName = value;
        else if (_stricmp(propertyName, "description") == 0 && tokenType == Json_String)
            description = value;
    }

    XJSONCloseReader(reader);

    if (FAILED(result) || version != 1)
    {
        displayName.clear();
        description.clear();
        DbgPrint("[codxe][T4 SP][UI] Invalid or unsupported usermap metadata: %s\n", path.c_str());
    }
}

bool IsSafeUsermapName(const char *name)
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

void SwapDDSHeader(DDSHeader &header)
{
    uint32_t *words = reinterpret_cast<uint32_t *>(&header);
    for (size_t i = 0; i < sizeof(header) / sizeof(uint32_t); ++i)
        words[i] = endian::ByteSwap(words[i]);
}

GfxImage *GetPreviewImage()
{
    const XAssetHeader header = DB_FindXAssetHeader(ASSET_TYPE_IMAGE, CODXE_PREVIEW_IMAGE, false, -1);
    GfxImage *image = header.image;
    if (image != previewImage)
    {
        previewImage = image;
        defaultPreviewPixels.clear();
    }
    return image;
}

bool GetPreviewStorage(GfxImage *image, unsigned char **destination, size_t *size)
{
    if (!image || image->mapType != MAPTYPE_2D || !image->texture.basemap || !image->pixels)
        return false;

    const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat);
    const uint32_t tiledSize = image::xenos_texture::CalculateTiledLevelSize(image->width, image->height, 0, format,
                                                                             image->texture.basemap->Format.Pitch);
    unsigned char *base = image::xenos_texture::GetTextureBase(image->texture.basemap, image->pixels);
    if (!base || tiledSize == 0)
        return false;

    *destination = base;
    *size = tiledSize;
    return true;
}

void RestoreDefaultPreview()
{
    GfxImage *image = GetPreviewImage();
    unsigned char *destination = nullptr;
    size_t destinationSize = 0;
    if (!GetPreviewStorage(image, &destination, &destinationSize) || defaultPreviewPixels.size() != destinationSize)
        return;

    XMemCpyStreaming_WriteCombined(destination, defaultPreviewPixels.data(), destinationSize);
}

bool ReplacePreview(const std::string &path)
{
    GfxImage *image = GetPreviewImage();
    unsigned char *destination = nullptr;
    size_t destinationSize = 0;
    if (!GetPreviewStorage(image, &destination, &destinationSize))
    {
        DbgPrint("[codxe][T4 SP][UI] Preview image asset is unavailable\n");
        return false;
    }

    if (defaultPreviewPixels.empty())
        defaultPreviewPixels.assign(destination, destination + destinationSize);

    const std::string file = filesystem::ReadFileToString(path);
    if (file.size() < sizeof(DDSHeader))
        return false;

    DDSHeader header;
    std::memcpy(&header, file.data(), sizeof(header));
    SwapDDSHeader(header);
    if (header.magic != DDS_MAGIC || header.size != DDS_HEADER_SIZE ||
        header.pixelFormat.size != DDS_PIXEL_FORMAT_SIZE || (header.pixelFormat.flags & DDS_FOURCC) == 0 ||
        header.pixelFormat.fourCC != DXT1_FOURCC || header.width != image->width || header.height != image->height)
    {
        DbgPrint("[codxe][T4 SP][UI] Preview must be %ux%u DXT1 DDS: %s\n", image->width, image->height, path.c_str());
        return false;
    }

    const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat);
    if (format != GPUTEXTUREFORMAT_DXT1)
        return false;

    const uint32_t linearSize = image::xenos_texture::CalculateLinearLevelSize(header.width, header.height, 0, format);
    const uint32_t rowPitch = image::xenos_texture::CalculateLinearRowPitch(header.width, 0, format);
    if (linearSize == 0 || rowPitch == 0 || file.size() < sizeof(DDSHeader) + linearSize)
        return false;

    std::vector<unsigned char> linear(linearSize);
    std::memcpy(linear.data(), file.data() + sizeof(DDSHeader), linearSize);
    image::xenos_texture::ApplyGpuEndian(linear.data(), linear.size(),
                                         static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

    std::vector<unsigned char> tiled(destinationSize);
    if (!image::xenos_texture::TileTextureLevel(image->width, image->height, 0, format,
                                                image->texture.basemap->Format.Pitch, tiled.data(), tiled.size(),
                                                linear.data(), linear.size(), rowPitch))
    {
        return false;
    }

    XMemCpyStreaming_WriteCombined(destination, tiled.data(), tiled.size());
    return true;
}

void SetMenuDvar(const char *name, const std::string &value)
{
    Dvar_SetFromStringByName(name, value.c_str());
}

void UpdateSelectedUsermap()
{
    if (selectedUsermap < 0 || selectedUsermap >= static_cast<int>(usermaps.size()))
    {
        SetMenuDvar("ui_codxe_usermap_name", "No custom maps found");
        SetMenuDvar("ui_codxe_usermap_description", "");
        SetMenuDvar("ui_codxe_usermap_count", "0");
        SetMenuDvar("ui_codxe_usermap_has_preview", "0");
        SetMenuDvar("ui_codxe_usermap_preview_hint", "Add preview.dds beside the map fastfile");
        RestoreDefaultPreview();
        return;
    }

    const UsermapEntry &entry = usermaps[selectedUsermap];
    char count[32];
    _snprintf_s(count, ARRAYSIZE(count), _TRUNCATE, "%u / %u", static_cast<unsigned int>(selectedUsermap + 1),
                static_cast<unsigned int>(usermaps.size()));
    SetMenuDvar("ui_codxe_usermap_name", entry.displayName);
    SetMenuDvar("ui_codxe_usermap_description", entry.description);
    SetMenuDvar("ui_codxe_usermap_count", count);
    SetMenuDvar("ui_codxe_usermap_preview_hint", std::string("Add preview.dds to _codxe/t4/usermaps/") + entry.name);

    const bool replaced = !entry.previewPath.empty() && ReplacePreview(entry.previewPath);
    SetMenuDvar("ui_codxe_usermap_has_preview", replaced ? "1" : "0");
    if (!replaced)
        RestoreDefaultPreview();
}

void ScanUsermaps()
{
    std::string selectedName;
    if (selectedUsermap >= 0 && selectedUsermap < static_cast<int>(usermaps.size()))
        selectedName = usermaps[selectedUsermap].name;

    usermaps.clear();
    selectedUsermap = 0;
    usermapsScanned = true;

    const std::string root = Config::ResolveDataDirectory(CODXE_USERMAPS_DIRECTORY);
    if (root.empty())
    {
        UpdateSelectedUsermap();
        DbgPrint("[codxe][T4 SP][UI] Usermap directory is unavailable\n");
        return;
    }

    WIN32_FIND_DATAA findData;
    const std::string pattern = filesystem::JoinPath(root.c_str(), "*");
    HANDLE findHandle = FindFirstFileA(pattern.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        UpdateSelectedUsermap();
        return;
    }

    do
    {
        const std::string name = findData.cFileName;
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 || !IsSafeUsermapName(name.c_str()) ||
            StartsWithIgnoreCase(name, "mp_"))
        {
            continue;
        }

        const std::string directory = filesystem::JoinPath(root.c_str(), name.c_str());
        const std::string fastfile = filesystem::JoinPath(directory.c_str(), (name + ".ff").c_str());
        if (!filesystem::FileExists(fastfile.c_str()))
            continue;

        UsermapEntry entry;
        entry.name = name;
        ReadMetadata(filesystem::JoinPath(directory.c_str(), "map.json"), entry.displayName, entry.description);
        if (entry.displayName.empty())
            entry.displayName = name;

        const std::string preview = filesystem::JoinPath(directory.c_str(), "preview.dds");
        if (filesystem::FileExists(preview.c_str()))
            entry.previewPath = preview;

        usermaps.push_back(entry);
    } while (FindNextFileA(findHandle, &findData));
    FindClose(findHandle);

    std::sort(usermaps.begin(), usermaps.end(), [](const UsermapEntry &left, const UsermapEntry &right)
              { return _stricmp(left.displayName.c_str(), right.displayName.c_str()) < 0; });

    const char *currentMap = Dvar_GetVariantString("ui_mapname");
    if (selectedName.empty() && currentMap)
        selectedName = currentMap;

    for (size_t i = 0; i < usermaps.size(); ++i)
    {
        if (_stricmp(usermaps[i].name.c_str(), selectedName.c_str()) == 0)
        {
            selectedUsermap = static_cast<int>(i);
            break;
        }
    }

    UpdateSelectedUsermap();
}

void EnsureUsermapsScanned()
{
    if (!usermapsScanned)
        ScanUsermaps();
}

void ActivateSelectedUsermap()
{
    EnsureUsermapsScanned();
    if (selectedUsermap < 0 || selectedUsermap >= static_cast<int>(usermaps.size()))
        return;

    const UsermapEntry &entry = usermaps[selectedUsermap];
    SetMenuDvar("ui_mapname", entry.name);
    SetMenuDvar("ui_preview_map", entry.name);
    SetMenuDvar("ui_codxe_usermap_mapname", entry.name);
    SetMenuDvar("ui_codxe_usermap_active", "1");
    usermapNameLookupLogged = false;
    usermapPreviewLookupLogged = false;
    SetMenuDvar("credits", "0");
    SetMenuDvar("credits_active", "0");

    DbgPrint("[codxe][T4 SP][UI] Activated usermap: name=%s mode=%s ui_mapname=%s active=%s\n", entry.name.c_str(),
             Dvar_GetVariantString("ui_codxe_usermap_mode"), Dvar_GetVariantString("ui_mapname"),
             Dvar_GetVariantString("ui_codxe_usermap_active"));

    if (_stricmp(Dvar_GetVariantString("ui_codxe_usermap_mode"), "solo") == 0)
    {
        char command[MAX_PATH];
        _snprintf_s(command, ARRAYSIZE(command), _TRUNCATE, "devmap %s\n", entry.name.c_str());
        Cbuf_AddText(0, command);
    }
    else
    {
        SetMenuDvar("arcademode", "0");
        SetMenuDvar("zombiemode", "1");
        SetMenuDvar("ui_hostOptionsEnabled", "0");
        Cbuf_AddText(0, "xupdatepartystate\n");
    }
}

int UI_FeederCount_Hook(int localClientNum, float feederID)
{
    if (feederID != CODXE_USERMAPS_FEEDER)
        return UI_FeederCount_Detour.GetOriginal<decltype(UI_FeederCount)>()(localClientNum, feederID);

    EnsureUsermapsScanned();
    return static_cast<int>(usermaps.size());
}

const char *UI_FeederItemText_Hook(int localClientNum, itemDef_s *item, float feederID, int index, unsigned int column,
                                   Material **handle)
{
    if (feederID != CODXE_USERMAPS_FEEDER)
    {
        return UI_FeederItemText_Detour.GetOriginal<decltype(UI_FeederItemText)>()(localClientNum, item, feederID,
                                                                                   index, column, handle);
    }

    if (handle)
        *handle = nullptr;
    EnsureUsermapsScanned();
    return index >= 0 && index < static_cast<int>(usermaps.size()) ? usermaps[index].displayName.c_str() : "";
}

void UI_FeederSelection_Hook(int localClientNum, float feederID, int index)
{
    if (feederID != CODXE_USERMAPS_FEEDER)
    {
        UI_FeederSelection_Detour.GetOriginal<decltype(UI_FeederSelection)>()(localClientNum, feederID, index);
        return;
    }

    EnsureUsermapsScanned();
    if (index < 0 || index >= static_cast<int>(usermaps.size()))
        return;

    selectedUsermap = index;
    UpdateSelectedUsermap();
}

const char *StringTable_Lookup_Hook(const StringTable *table, int comparisonColumn, const char *value,
                                    int valueColumn)
{
    if (table && table->name && value && comparisonColumn == 0 &&
        _stricmp(table->name, "maps/mapsTable.csv") == 0 &&
        std::strcmp(Dvar_GetVariantString("ui_codxe_usermap_active"), "1") == 0 &&
        _stricmp(value, Dvar_GetVariantString("ui_codxe_usermap_mapname")) == 0)
    {
        if (valueColumn == 3)
        {
            if (!usermapNameLookupLogged)
            {
                DbgPrint("[codxe][T4 SP][UI] Supplying virtual mapsTable name for %s\n", value);
                usermapNameLookupLogged = true;
            }
            return Dvar_GetVariantString("ui_codxe_usermap_name");
        }
        if (valueColumn == 4)
        {
            if (!usermapPreviewLookupLogged)
            {
                DbgPrint("[codxe][T4 SP][UI] Supplying virtual mapsTable preview for %s\n", value);
                usermapPreviewLookupLogged = true;
            }
            return CODXE_PREVIEW_MATERIAL;
        }
    }

    return StringTable_Lookup_Detour.GetOriginal<decltype(StringTable_Lookup)>()(table, comparisonColumn, value,
                                                                                 valueColumn);
}

} // namespace

void DrawBranding(int localClientNum)
{
    static auto consoleFont = R_RegisterFont("fonts/consoleFont", -1);
    const char *text = branding::GetBrandingString();
    const float color_white_rgba[4] = {1.0f, 1.0f, 1.0f, 0.5f};
    UI_DrawText(scrPlaceFullUnsafe, text, 64, consoleFont, -100.f, 10.f, 0, 0, 0.2, color_white_rgba, 0);
}

Detour UI_Refresh_Detour;

void UI_Refresh_Hook(int localClientNum)
{
    UI_Refresh_Detour.GetOriginal<decltype(UI_Refresh)>()(localClientNum);

    // Menu files can request activation using stock setdvar handling, avoiding a custom UI script command.
    if (std::strcmp(Dvar_GetVariantString("ui_codxe_usermap_activate"), "1") == 0)
    {
        SetMenuDvar("ui_codxe_usermap_activate", "0");
        ActivateSelectedUsermap();
    }

    console::OnUIRefresh();
    DrawBranding(localClientNum);
}

void Campaign_UnlockAll()
{
    Cbuf_AddText(0, "set mis_01 50\n");
}

Detour Menus_OpenByName_Detour;
Detour Item_Slider_HandleKey_Detour;

// Stock sliders move by 5% of their range; FOV controls need finer, predictable steps.
int Item_Slider_HandleKey_Hook(UiContext *dc, itemDef_s *item, int key)
{
    const auto original = Item_Slider_HandleKey_Detour.GetOriginal<decltype(Item_Slider_HandleKey)>();

    int direction = 0;
    if (key == K_DPAD_LEFT || key == K_APAD_LEFT || key == K_LEFTARROW || key == K_PGUP)
        direction = -1;
    else if (key == K_DPAD_RIGHT || key == K_APAD_RIGHT || key == K_RIGHTARROW || key == K_PGDN)
        direction = 1;

    if (!item || !direction)
        return original(dc, item, key);

    const auto *dvarName = item->dvar;
    if (!dvarName)
        return original(dc, item, key);

    float step;
    if (std::strcmp(dvarName, "cg_fov") == 0)
        step = 1.0f;
    else if (std::strcmp(dvarName, "cg_fovScale") == 0)
        step = 0.05f;
    else
        return original(dc, item, key);

    const auto *limits = item->typeData.editField;
    if (!limits)
        return original(dc, item, key);

    const float before = static_cast<float>(std::atof(Dvar_GetVariantString(dvarName)));
    const int handled = original(dc, item, key);
    if (handled)
    {
        float after = before + static_cast<float>(direction) * step;
        if (after < limits->minVal)
            after = limits->minVal;
        else if (after > limits->maxVal)
            after = limits->maxVal;

        char value[32];
        sprintf_s(value, "%g", after);
        Dvar_SetFromStringByName(dvarName, value);
    }

    return handled;
}

void Menus_OpenByName_Hook(UiContext *dc, const char *menuName)
{
    EnsureUsermapDvarsRegistered();

    if (std::strcmp(menuName, "codxe_usermaps") == 0)
        ScanUsermaps();

    if (std::strcmp(menuName, "main_solo") == 0 || std::strcmp(menuName, "main_online") == 0)
        Campaign_UnlockAll();

    Menus_OpenByName_Detour.GetOriginal<decltype(Menus_OpenByName)>()(dc, menuName);
}

ui::ui()
{
    UI_Refresh_Detour = Detour(UI_Refresh, UI_Refresh_Hook);
    UI_Refresh_Detour.Install();

    Menus_OpenByName_Detour = Detour(Menus_OpenByName, Menus_OpenByName_Hook);
    Menus_OpenByName_Detour.Install();

    Item_Slider_HandleKey_Detour = Detour(Item_Slider_HandleKey, Item_Slider_HandleKey_Hook);
    Item_Slider_HandleKey_Detour.Install();

    UI_FeederCount_Detour = Detour(UI_FeederCount, UI_FeederCount_Hook);
    UI_FeederCount_Detour.Install();

    UI_FeederItemText_Detour = Detour(UI_FeederItemText, UI_FeederItemText_Hook);
    UI_FeederItemText_Detour.Install();

    UI_FeederSelection_Detour = Detour(UI_FeederSelection, UI_FeederSelection_Hook);
    UI_FeederSelection_Detour.Install();

    StringTable_Lookup_Detour = Detour(StringTable_Lookup, StringTable_Lookup_Hook);
    StringTable_Lookup_Detour.Install();
}

ui::~ui()
{
    StringTable_Lookup_Detour.Remove();
    UI_FeederSelection_Detour.Remove();
    UI_FeederItemText_Detour.Remove();
    UI_FeederCount_Detour.Remove();
    Item_Slider_HandleKey_Detour.Remove();
    Menus_OpenByName_Detour.Remove();
    UI_Refresh_Detour.Remove();

    usermaps.clear();
    defaultPreviewPixels.clear();
    selectedUsermap = 0;
    usermapsScanned = false;
    usermapDvarsRegistered = false;
    usermapNameLookupLogged = false;
    usermapPreviewLookupLogged = false;
    previewImage = nullptr;
}
} // namespace sp
} // namespace t4
