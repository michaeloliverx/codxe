#include "pch.h"
#include "arenas.h"
#include "usermaps.h"

#include "common/arena.h"
#include "common/usermaps.h"

namespace iw4
{
namespace mp_tu6
{
namespace
{
struct CachedArena
{
    bool valid;
    arena::Fields fields;
    CachedArena() : valid(false)
    {
    }
};

std::map<std::string, CachedArena> cache;
CRITICAL_SECTION cacheLock;
Detour UI_GetCurrentMapCustom_Detour;
Detour UI_GetMapDisplayName_Detour;
Detour UI_GetCurrentMapName_Detour;
Detour UI_GetCurrentMapImage_Detour;

class CacheGuard
{
  public:
    CacheGuard()
    {
        EnterCriticalSection(&cacheLock);
    }
    ~CacheGuard()
    {
        LeaveCriticalSection(&cacheLock);
    }
};

void Load(const char *map, CachedArena &entry)
{
    const std::string path = usermaps::FastfilePath(map);
    if (path.empty() || !filesystem::FileExists(path.c_str()))
        return;
    const std::string directory = filesystem::JoinPath(usermaps::GetDirectory(), map);
    entry.valid = usermaps::ReadMetadata(directory, map, entry.fields);
}

std::string Translate(const std::string &value)
{
    const char *key = value.c_str();
    if (*key == '@' || *key == '\x15')
        ++key;
    else if (value.find('_') == std::string::npos ||
             value.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != std::string::npos)
        return value;
    const char *translated = *key ? SEH_StringEd_GetString(key) : nullptr;
    return translated && *translated ? translated : key;
}

const char *GetCurrentMapCustom(const char *field)
{
    if (field && _stricmp(field, "mapimage") == 0 && Usermaps::EnsurePreview(GetDvarString("ui_mapname")))
        return "$codxe_usermap_preview";
    std::string value;
    if (Arenas::GetField(GetDvarString("ui_mapname"), field, value))
    {
        if (_stricmp(field, "mapimage") == 0 && value.empty())
            return "logo_cod2";
        return va("%s", value.c_str());
    }
    return UI_GetCurrentMapCustom_Detour.GetOriginal<UI_GetCurrentMapCustom_t>()(field);
}

const char *GetMapDisplayName(const char *map)
{
    std::string value;
    if (Arenas::GetField(map, "longname", value) && !value.empty())
        return va("%s", Translate(value).c_str());
    return UI_GetMapDisplayName_Detour.GetOriginal<UI_GetMapDisplayName_t>()(map);
}

const char *GetCurrentMapName()
{
    // Both the offline lobby and connect.menu use getmapname(), which has its
    // own arena lookup rather than calling UI_GetMapDisplayName.
    std::string value;
    if (Arenas::GetField(GetDvarString("ui_mapname"), "longname", value) && !value.empty())
        return va("%s", Translate(value).c_str());
    return UI_GetCurrentMapName_Detour.GetOriginal<UI_GetCurrentMapName_t>()();
}

const char *GetCurrentMapImage()
{
    const char *map = GetDvarString("ui_mapname");
    if (Usermaps::EnsurePreview(map))
        return "$codxe_usermap_preview";
    std::string value;
    if (Arenas::GetField(map, "mapimage", value))
        return "logo_cod2";
    return UI_GetCurrentMapImage_Detour.GetOriginal<UI_GetCurrentMapImage_t>()();
}
} // namespace

void Arenas::Reload()
{
    CacheGuard guard;
    cache.clear();
}

bool Arenas::GetField(const char *map, const char *field, std::string &value)
{
    value.clear();
    if (!usermaps::IsMultiplayerName(map) || !field || !*field)
        return false;
    CacheGuard guard;
    const std::string name = arena::Lower(map);
    std::map<std::string, CachedArena>::iterator found = cache.find(name);
    if (found == cache.end())
    {
        found = cache.insert(std::make_pair(name, CachedArena())).first;
        Load(map, found->second);
    }
    if (!found->second.valid)
        return false;
    const arena::Fields::const_iterator entry = found->second.fields.find(arena::Lower(field));
    if (entry != found->second.fields.end())
        value = entry->second;
    return true;
}

std::string Arenas::DisplayName(const char *map)
{
    std::string value;
    return GetField(map, "longname", value) && !value.empty() ? Translate(value) : map;
}

std::string Arenas::Description(const char *map)
{
    std::string value;
    return GetField(map, "description", value) ? Translate(value) : "";
}

Arenas::Arenas()
{
    InitializeCriticalSection(&cacheLock);
    UI_GetCurrentMapCustom_Detour = Detour(UI_GetCurrentMapCustom, GetCurrentMapCustom);
    UI_GetCurrentMapCustom_Detour.Install();
    UI_GetMapDisplayName_Detour = Detour(UI_GetMapDisplayName, GetMapDisplayName);
    UI_GetMapDisplayName_Detour.Install();
    UI_GetCurrentMapName_Detour = Detour(UI_GetCurrentMapName, GetCurrentMapName);
    UI_GetCurrentMapName_Detour.Install();
    UI_GetCurrentMapImage_Detour = Detour(UI_GetCurrentMapImage, GetCurrentMapImage);
    UI_GetCurrentMapImage_Detour.Install();
}

Arenas::~Arenas()
{
    UI_GetCurrentMapImage_Detour.Remove();
    UI_GetCurrentMapName_Detour.Remove();
    UI_GetMapDisplayName_Detour.Remove();
    UI_GetCurrentMapCustom_Detour.Remove();
    cache.clear();
    DeleteCriticalSection(&cacheLock);
}
} // namespace mp_tu6
} // namespace iw4
