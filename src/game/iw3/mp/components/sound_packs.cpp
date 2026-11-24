#include "pch.h"
#include "sound_packs.h"

#include "common/usermaps.h"
#include "mods.h"

namespace iw3
{
namespace mp
{
struct StreamFileName
{
    unsigned index;
    union {
        struct {
            const char *dir;
            const char *name;
        } raw;
        struct {
            unsigned offset;
            unsigned length;
        } packed;
    };
};

struct StreamFile
{
    HANDLE handle;
    int length;
    int startOffset;
};

static_assert(sizeof(StreamFileName) == 12, "");
static_assert(sizeof(StreamFile) == 12, "");

struct CacheFileName
{
    unsigned index;
    union {
        char raw[128];
        struct {
            unsigned offset;
            unsigned length;
        } packed;
    };
};

static_assert(sizeof(CacheFileName) == 132, "");

namespace
{
// OAT writes these exact owner IDs. Testing the tag bit alone would also match stock loose DLC (-1).
const unsigned CODEXE_PACK_TAG = 1u << 30;
const unsigned MOD_INDEX = CODEXE_PACK_TAG;
const unsigned MAP_INDEX = CODEXE_PACK_TAG | 1u;

// Runtime mount IDs distinguish packs in the stream cache across mod and map changes.
const unsigned FIRST_MOUNT_INDEX = CODEXE_PACK_TAG | 0x10000u;
const unsigned LAST_PRIVATE_INDEX = CODEXE_PACK_TAG | 0x0FFFFFFFu;
const unsigned SECTOR_SIZE = 2048;

struct Mount
{
    std::string path;
    HANDLE handle;
    unsigned size;
    unsigned streams;
};

Detour StreamFileNameGetCacheName_Detour;
Detour StreamFileOpen_Detour;
Detour StreamFileClose_Detour;
CRITICAL_SECTION mountsLock;
std::vector<Mount> mounts;
std::map<StreamFile *, unsigned> openStreams;
unsigned activeMounts[2] = {};

class MountsGuard
{
  public:
    MountsGuard() { EnterCriticalSection(&mountsLock); }
    ~MountsGuard() { LeaveCriticalSection(&mountsLock); }
};

std::string GetPackPath(unsigned role)
{
    if (role == MOD_INDEX)
        return ModList::ResolvePath("mod.pak");

    const char *mapName = Dvar_GetString("mapname");
    if (!usermaps::IsMultiplayerName(mapName))
        return std::string();
    const std::string mapFastfile = usermaps::FastfilePath(mapName);
    const size_t separator = mapFastfile.find_last_of("\\/");
    if (separator == std::string::npos)
        return std::string();
    return mapFastfile.substr(0, separator + 1) + mapName + ".pak";
}

void *StreamFileNameGetCacheName_Hook(const StreamFileName *source, CacheFileName *destination)
{
    if (source->index != MOD_INDEX && source->index != MAP_INDEX)
        return StreamFileNameGetCacheName_Detour.GetOriginal<decltype(StreamFileNameGetCacheName)>()(source, destination);

    const std::string path = GetPackPath(source->index);
    MountsGuard guard;
    unsigned &active = activeMounts[source->index - MOD_INDEX];
    destination->index = source->index;
    destination->packed.offset = source->packed.offset;
    destination->packed.length = source->packed.length;
    if (path.empty())
    {
        active = 0;
        return destination;
    }

    if (!active || _stricmp(mounts[active - FIRST_MOUNT_INDEX].path.c_str(), path.c_str()) != 0)
    {
        // Keep IDs for queued cache keys even after another map becomes active.
        // Only paths are retained once the last stream releases the handle.
        if (mounts.size() >= 4096)
        {
            DbgPrint("[codxe][IW3][SoundPaks] Mount identity limit reached\n");
            return destination;
        }
        Mount mount = {path, INVALID_HANDLE_VALUE, 0, 0};
        mounts.push_back(mount);
        active = FIRST_MOUNT_INDEX + static_cast<unsigned>(mounts.size()) - 1;
    }
    destination->index = active;
    return destination;
}

void ReleaseMount(Mount &mount)
{
    if (mount.handle != INVALID_HANDLE_VALUE && !mount.streams)
    {
        CloseHandle(mount.handle);
        mount.handle = INVALID_HANDLE_VALUE;
        mount.size = 0;
    }
}

HANDLE StreamFileOpen_Hook(StreamFile *stream, const CacheFileName *name)
{
    if (name->index < MOD_INDEX || name->index > LAST_PRIVATE_INDEX)
        return StreamFileOpen_Detour.GetOriginal<decltype(StreamFileOpen)>()(stream, name);

    stream->handle = INVALID_HANDLE_VALUE;
    stream->length = 0;
    stream->startOffset = 0;
    MountsGuard guard;
    if (name->index < FIRST_MOUNT_INDEX || name->index - FIRST_MOUNT_INDEX >= mounts.size())
    {
        DbgPrint("[codxe][IW3][SoundPaks] No pack for stream index %08X\n", name->index);
        return stream->handle;
    }

    Mount &mount = mounts[name->index - FIRST_MOUNT_INDEX];
    const bool newHandle = mount.handle == INVALID_HANDLE_VALUE;
    if (newHandle)
    {
        mount.handle = CreateFileA(mount.path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                   FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, nullptr);
        if (mount.handle == INVALID_HANDLE_VALUE)
        {
            DbgPrint("[codxe][IW3][SoundPaks] Cannot open %s (%u)\n", mount.path.c_str(), GetLastError());
            return stream->handle;
        }
        DWORD high = 0;
        const DWORD size = GetFileSize(mount.handle, &high);
        if (!high && size <= INT_MAX && size % SECTOR_SIZE == 0)
            mount.size = size;
    }

    const unsigned offset = name->packed.offset;
    const unsigned length = name->packed.length;
    if (offset < SECTOR_SIZE || offset % SECTOR_SIZE != 0 || !length ||
        offset >= mount.size || length > mount.size - offset)
    {
        DbgPrint("[codxe][IW3][SoundPaks] Invalid range %u+%u in %s\n", offset, length, mount.path.c_str());
        ReleaseMount(mount);
        return stream->handle;
    }

    openStreams[stream] = name->index;
    ++mount.streams;
    stream->handle = mount.handle;
    stream->length = static_cast<int>(length);
    stream->startOffset = static_cast<int>(offset);
    if (newHandle)
        DbgPrint("[codxe][IW3][SoundPaks] Opened %s (id %08X)\n", mount.path.c_str(), name->index);
    return stream->handle;
}

HANDLE StreamFileClose_Hook(StreamFile *stream)
{
    {
        MountsGuard guard;
        const auto entry = openStreams.find(stream);
        if (entry != openStreams.end())
        {
            Mount &mount = mounts[entry->second - FIRST_MOUNT_INDEX];
            openStreams.erase(entry);
            --mount.streams;
            ReleaseMount(mount);
            stream->handle = nullptr;
            stream->length = 0;
            stream->startOffset = 0;
            return nullptr;
        }
    }
    return StreamFileClose_Detour.GetOriginal<decltype(StreamFileClose)>()(stream);
}

} // namespace

SoundPacks::SoundPacks()
{
    InitializeCriticalSection(&mountsLock);
    StreamFileNameGetCacheName_Detour = Detour(StreamFileNameGetCacheName, StreamFileNameGetCacheName_Hook);
    StreamFileOpen_Detour = Detour(StreamFileOpen, StreamFileOpen_Hook);
    StreamFileClose_Detour = Detour(StreamFileClose, StreamFileClose_Hook);
    StreamFileClose_Detour.Install();
    StreamFileOpen_Detour.Install();
    StreamFileNameGetCacheName_Detour.Install();
}
} // namespace mp
} // namespace iw3
