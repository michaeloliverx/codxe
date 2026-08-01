#include "pch.h"
#include "fastfiles.h"

namespace iw4
{
namespace mp_tu6
{
namespace
{
const char *const CODXE_PATCH_ZONE = "codxe_patch_mp";
const char *const CODXE_PATCH_FILENAME = "codxe_patch_mp.ff";
const char *const CODXE_PATCH_RELATIVE_PATH = "_codxe\\zone\\codxe_patch_mp.ff";
const char *const CODXE_PATCH_PATH = "game:\\_codxe\\zone\\codxe_patch_mp.ff";
const char *const CODXE_UI_ZONE = "codxe_ui_mp";
const char *const CODXE_UI_FILENAME = "codxe_ui_mp.ff";
const char *const CODXE_UI_RELATIVE_PATH = "_codxe\\zone\\codxe_ui_mp.ff";
const char *const CODXE_UI_PATH = "game:\\_codxe\\zone\\codxe_ui_mp.ff";
const char *const CODXE_ZONE_RELATIVE_DIRECTORY = "_codxe\\zone\\";
const char *const CODXE_IMAGEFILE_FILENAME = "imagefile5.pak";
const char *const GAME_DEVICE_PREFIX = "game:\\";
const char *const FASTFILE_EXTENSION = ".ff";
const unsigned int MAX_ZONE_COUNT = 32;

struct XZoneInfo
{
    const char *name;
    int allocFlags;
    int freeFlags;
};

struct SysFile
{
    void *handle;
    int startOffset;
};

static_assert(sizeof(XZoneInfo) == 12, "XZoneInfo size mismatch");
static_assert(sizeof(SysFile) == 8, "SysFile size mismatch");

typedef void (*DB_LoadXAssets_t)(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
typedef SysFile (*Sys_CreateFile_t)(const char *dir, const char *filename);
typedef float (*DB_GetLoadedFraction_t)(int, int, int, __int64);
typedef void (*Party_Frame_t)(void *party, unsigned int localControllerIndex, int msec, int, __int64);
typedef void (*Cbuf_AddText_t)(int localClientNum, const char *text);
typedef void (*SV_SpawnServer_t)(char *server, int mapIsPreloaded, int savegame, bool cheats);
typedef int (*CL_CheckTimeout_t)(int localClientNum);

auto DB_LoadXAssets = reinterpret_cast<DB_LoadXAssets_t>(0x821E1C60);
auto Sys_CreateFile = reinterpret_cast<Sys_CreateFile_t>(0x82319928);
auto DB_GetLoadedFraction = reinterpret_cast<DB_GetLoadedFraction_t>(0x821B0120);
auto Party_Frame = reinterpret_cast<Party_Frame_t>(0x8219FBB8);
auto Cbuf_AddText_Internal = reinterpret_cast<Cbuf_AddText_t>(0x82275C60);
auto SV_SpawnServer = reinterpret_cast<SV_SpawnServer_t>(0x822C0AC0);
auto CL_CheckTimeout = reinterpret_cast<CL_CheckTimeout_t>(0x82184E10);

Detour DB_LoadXAssets_Detour;
Detour Sys_CreateFile_Detour;
Detour Party_Frame_Detour;
Detour Cbuf_AddText_Detour;
Detour SV_SpawnServer_Detour;
Detour CL_CheckTimeout_Detour;

// Retail TU6 PartyData offsets used by the vanilla map-preload handoff in Party_Frame.
const std::size_t PARTY_PRELOADING_MAP_OFFSET = 5329;
const std::size_t PARTY_ARE_WE_HOST_OFFSET = 5340;
const std::size_t PARTY_ID_OFFSET = 5428;

// Retail TU6 globals used by DB_GetLoadedFraction.
const std::uintptr_t DB_LOADED_EXTERNAL_BYTES_ADDRESS = 0x825CE4C0;
const std::uintptr_t DB_TOTAL_SIZE_ADDRESS = 0x825CE4DC;
const std::uintptr_t DB_TOTAL_EXTERNAL_BYTES_ADDRESS = 0x825F84F0;
const std::uintptr_t DB_LOADED_SIZE_ADDRESS = 0x825F850C;

// Retail TU6 client networking globals and structure layout used by CL_CheckTimeout.
const std::uintptr_t CLIENT_MIGRATION_STATE_ADDRESS = 0x825A5920;
const std::uintptr_t CLIENT_CONNECTIONS_ADDRESS = 0x82508990;
const std::uintptr_t CLIENTS_ADDRESS = 0x825A8B6C;
const std::uintptr_t CLIENT_STATIC_REALTIME_ADDRESS = 0x82508B1C;
const std::uintptr_t CL_MAX_LOCAL_CLIENTS_ADDRESS = 0x825089C8;
const std::uintptr_t CL_CONNECT_TIMEOUT_ADDRESS = 0x825089A4;
const std::uintptr_t CL_TIMEOUT_ADDRESS = 0x825089AC;
const std::uintptr_t CL_CONNECTION_ATTEMPTS_ADDRESS = 0x825089CC;
const std::uintptr_t COM_SV_RUNNING_ADDRESS = 0x830F08E4;
const std::uintptr_t CL_PAUSED_ADDRESS = 0x830F08D0;
const std::uintptr_t SV_PAUSED_ADDRESS = 0x830F1A68;
const std::size_t CLIENT_UI_ACTIVE_SIZE = 3216;
const std::size_t CLIENT_CONNECTION_SIZE = 333160;
const std::size_t CLIENT_CONNECTION_LAST_PACKET_TIME_OFFSET = 12;
const std::size_t CLIENT_CONNECTION_PACKET_COUNT_OFFSET = 32;
const unsigned int TIMEOUT_LOG_INTERVAL_MS = 1000;
const int MAX_TRACKED_LOCAL_CLIENTS = 4;

const char *GetUiMapName()
{
    auto uiMapName = *reinterpret_cast<dvar_t **>(0x8369DE38);
    if (!uiMapName || !uiMapName->current.string)
        return "(null)";

    return uiMapName->current.string;
}

const char *GetConnectionStateName(int connectionState)
{
    switch (connectionState)
    {
    case CA_DISCONNECTED:
        return "disconnected";
    case CA_CINEMATIC:
        return "cinematic";
    case CA_LOGO:
        return "logo";
    case CA_CONNECTING:
        return "connecting";
    case CA_CHALLENGING:
        return "challenging";
    case CA_CONNECTED:
        return "connected";
    case CA_SENDINGSTATS:
        return "sendingstats";
    case CA_LOADING:
        return "loading";
    case CA_PRIMED:
        return "primed";
    case CA_ACTIVE:
        return "active";
    default:
        return "unknown";
    }
}

void LogZoneBatch(const char *label, const XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
{
    DbgPrint("[codxe][fastfiles] %s DB_LoadXAssets batch: count=%u sync=%d\n", label, zoneCount, sync);

    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        DbgPrint("[codxe][fastfiles]   zone[%u]: name=%s alloc=0x%08X free=0x%08X\n", i,
                 zoneInfo[i].name ? zoneInfo[i].name : "(null)", zoneInfo[i].allocFlags, zoneInfo[i].freeFlags);
    }
}

const XZoneInfo *FindZone(const XZoneInfo *zoneInfo, unsigned int zoneCount, const char *name)
{
    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        if (zoneInfo[i].name && std::strcmp(zoneInfo[i].name, name) == 0)
            return &zoneInfo[i];
    }

    return nullptr;
}

bool IsSafeFastfileName(const char *filename)
{
    if (!filename)
        return false;

    const std::size_t filenameLength = std::strlen(filename);
    const std::size_t extensionLength = std::strlen(FASTFILE_EXTENSION);

    if (filenameLength <= extensionLength ||
        std::strcmp(filename + filenameLength - extensionLength, FASTFILE_EXTENSION) != 0)
    {
        return false;
    }

    // Zone names are restricted to safe filename characters so an incoming
    // name can never escape the Codxe zone directory.
    for (std::size_t i = 0; i < filenameLength - extensionLength; ++i)
    {
        const char c = filename[i];
        const bool isLetter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        const bool isDigit = c >= '0' && c <= '9';
        if (!isLetter && !isDigit && c != '_' && c != '-')
            return false;
    }

    return true;
}

void DB_LoadXAssets_Hook(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
{
    auto original = DB_LoadXAssets_Detour.GetOriginal<DB_LoadXAssets_t>();
    const unsigned int loadStart = Sys_Milliseconds();

    DbgPrint("[codxe][fastfiles] DB_LoadXAssets begin: t=%u map=%s count=%u sync=%d\n", loadStart, GetUiMapName(),
             zoneCount, sync);
    const bool patchFileExists = FileExists(CODXE_PATCH_PATH);
    const bool uiFileExists = FileExists(CODXE_UI_PATH);
    const XZoneInfo *stockPatchZone = FindZone(zoneInfo, zoneCount, "patch_mp");
    const XZoneInfo *stockUiZone = FindZone(zoneInfo, zoneCount, "ui_mp");
    const bool injectPatch = patchFileExists && stockPatchZone;
    const bool injectUi = uiFileExists && stockUiZone;
    const unsigned int injectionCount = (injectPatch ? 1 : 0) + (injectUi ? 1 : 0);
    const bool hasCapacity = zoneCount <= MAX_ZONE_COUNT && injectionCount <= MAX_ZONE_COUNT - zoneCount;

    LogZoneBatch("incoming", zoneInfo, zoneCount, sync);
    DbgPrint("[codxe][fastfiles] Injection check: patch_file=%s stock_patch=%s ui_file=%s stock_ui=%s capacity=%s\n",
             patchFileExists ? "present" : "missing", stockPatchZone ? "found" : "not-found",
             uiFileExists ? "present" : "missing", stockUiZone ? "found" : "not-found",
             hasCapacity ? "available" : "full");

    if (injectionCount == 0 || !hasCapacity)
    {
        DbgPrint("[codxe][fastfiles] Passing DB_LoadXAssets batch through unchanged\n");
        original(zoneInfo, zoneCount, sync);
        const unsigned int loadEnd = Sys_Milliseconds();
        DbgPrint("[codxe][fastfiles] DB_LoadXAssets returned: t=%u elapsed=%ums map=%s\n", loadEnd, loadEnd - loadStart,
                 GetUiMapName());
        return;
    }

    XZoneInfo zones[MAX_ZONE_COUNT];
    unsigned int outputZoneCount = 0;

    if (injectUi)
    {
        DbgPrint("[codxe][fastfiles] ui_mp flags: alloc=0x%08X free=0x%08X\n", stockUiZone->allocFlags,
                 stockUiZone->freeFlags);
    }

    for (unsigned int i = 0; i < zoneCount; ++i)
    {
        if (injectUi && zoneInfo[i].name && std::strcmp(zoneInfo[i].name, "ui_mp") == 0)
        {
            zones[outputZoneCount].name = CODXE_UI_ZONE;
            zones[outputZoneCount].allocFlags = stockUiZone->allocFlags;
            zones[outputZoneCount].freeFlags = stockUiZone->freeFlags;
            ++outputZoneCount;

            DbgPrint("[codxe][fastfiles] Loading %s before the stock UI zone\n", CODXE_UI_ZONE);
        }

        zones[outputZoneCount] = zoneInfo[i];
        ++outputZoneCount;
    }

    if (injectPatch)
    {
        DbgPrint("[codxe][fastfiles] patch_mp flags: alloc=0x%08X free=0x%08X\n", stockPatchZone->allocFlags,
                 stockPatchZone->freeFlags);

        zones[outputZoneCount].name = CODXE_PATCH_ZONE;
        zones[outputZoneCount].allocFlags = stockPatchZone->allocFlags;
        zones[outputZoneCount].freeFlags = stockPatchZone->freeFlags;
        ++outputZoneCount;

        DbgPrint("[codxe][fastfiles] Loading %s after the stock patch zones\n", CODXE_PATCH_ZONE);
    }

    LogZoneBatch("forwarded", zones, outputZoneCount, sync);
    original(zones, outputZoneCount, sync);
    const unsigned int loadEnd = Sys_Milliseconds();
    DbgPrint("[codxe][fastfiles] DB_LoadXAssets returned: t=%u elapsed=%ums map=%s\n", loadEnd, loadEnd - loadStart,
             GetUiMapName());
}

SysFile Sys_CreateFile_Hook(const char *dir, const char *filename)
{
    auto original = Sys_CreateFile_Detour.GetOriginal<Sys_CreateFile_t>();

    if (filename && std::strcmp(filename, CODXE_PATCH_FILENAME) == 0)
    {
        DbgPrint("[codxe][fastfiles] Redirecting %s to %s (dir=%s)\n", CODXE_PATCH_FILENAME, CODXE_PATCH_RELATIVE_PATH,
                 dir ? dir : "(null)");
        return original(dir, CODXE_PATCH_RELATIVE_PATH);
    }

    if (filename && std::strcmp(filename, CODXE_UI_FILENAME) == 0)
    {
        DbgPrint("[codxe][fastfiles] Redirecting %s to %s (dir=%s)\n", CODXE_UI_FILENAME, CODXE_UI_RELATIVE_PATH,
                 dir ? dir : "(null)");
        return original(dir, CODXE_UI_RELATIVE_PATH);
    }

    if (filename && std::strcmp(filename, CODXE_IMAGEFILE_FILENAME) == 0)
    {
        const std::string relativePath = std::string(CODXE_ZONE_RELATIVE_DIRECTORY) + filename;
        const std::string devicePath = std::string(GAME_DEVICE_PREFIX) + relativePath;

        if (FileExists(devicePath.c_str()))
        {
            DbgPrint("[codxe][fastfiles] Redirecting Codxe imagefile %s to %s (dir=%s)\n", filename,
                     relativePath.c_str(), dir ? dir : "(null)");
            return original(dir, relativePath.c_str());
        }
    }

    if (IsSafeFastfileName(filename))
    {
        const std::string relativePath = std::string(CODXE_ZONE_RELATIVE_DIRECTORY) + filename;
        const std::string devicePath = std::string(GAME_DEVICE_PREFIX) + relativePath;

        if (FileExists(devicePath.c_str()))
        {
            DbgPrint("[codxe][fastfiles] Redirecting Codxe zone %s to %s (dir=%s)\n", filename, relativePath.c_str(),
                     dir ? dir : "(null)");
            return original(dir, relativePath.c_str());
        }
    }

    return original(dir, filename);
}

int CL_CheckTimeout_Hook(int localClientNum)
{
    auto original = CL_CheckTimeout_Detour.GetOriginal<CL_CheckTimeout_t>();
    const int maxLocalClients = *reinterpret_cast<int *>(CL_MAX_LOCAL_CLIENTS_ADDRESS);

    if (localClientNum < 0 || localClientNum >= maxLocalClients || localClientNum >= MAX_TRACKED_LOCAL_CLIENTS)
    {
        return original(localClientNum);
    }

    const int connectionState = clientUIActives[localClientNum].connectionState;
    const auto clientOffset = static_cast<std::size_t>(localClientNum);
    const int migrationState =
        *reinterpret_cast<int *>(CLIENT_MIGRATION_STATE_ADDRESS + CLIENT_UI_ACTIVE_SIZE * clientOffset);
    const int realtime = *reinterpret_cast<int *>(CLIENT_STATIC_REALTIME_ADDRESS);

    auto clientConnections = *reinterpret_cast<unsigned char **>(CLIENT_CONNECTIONS_ADDRESS);
    auto activeClients = *reinterpret_cast<clientActive_t **>(CLIENTS_ADDRESS);
    auto clConnectTimeout = *reinterpret_cast<dvar_t **>(CL_CONNECT_TIMEOUT_ADDRESS);
    auto clTimeout = *reinterpret_cast<dvar_t **>(CL_TIMEOUT_ADDRESS);
    auto clConnectionAttempts = *reinterpret_cast<dvar_t **>(CL_CONNECTION_ATTEMPTS_ADDRESS);
    auto comSvRunning = *reinterpret_cast<dvar_t **>(COM_SV_RUNNING_ADDRESS);
    auto clPaused = *reinterpret_cast<dvar_t **>(CL_PAUSED_ADDRESS);
    auto svPaused = *reinterpret_cast<dvar_t **>(SV_PAUSED_ADDRESS);

    int lastPacketTime = 0;
    int connectPacketCount = 0;
    if (clientConnections)
    {
        auto connection = clientConnections + CLIENT_CONNECTION_SIZE * clientOffset;
        lastPacketTime = *reinterpret_cast<int *>(connection + CLIENT_CONNECTION_LAST_PACKET_TIME_OFFSET);
        connectPacketCount = *reinterpret_cast<int *>(connection + CLIENT_CONNECTION_PACKET_COUNT_OFFSET);
    }

    const int timeoutCount = activeClients ? activeClients[localClientNum].timeoutcount : 0;
    const int silenceMs = lastPacketTime > 0 ? realtime - lastPacketTime : -1;
    const float connectTimeoutSeconds = clConnectTimeout ? clConnectTimeout->current.value : -1.0f;
    const float activeTimeoutSeconds = clTimeout ? clTimeout->current.value : -1.0f;
    const int connectionAttemptLimit = clConnectionAttempts ? clConnectionAttempts->current.integer : -1;
    const bool serverRunning = comSvRunning && comSvRunning->current.enabled;
    const int connectTimeoutMs = connectTimeoutSeconds >= 0.0f ? static_cast<int>(connectTimeoutSeconds * 1000.0f) : -1;
    const int activeTimeoutMs = activeTimeoutSeconds >= 0.0f ? static_cast<int>(activeTimeoutSeconds * 1000.0f) : -1;
    const bool clientPaused = clPaused && clPaused->current.integer != 0;
    const bool serverPaused = svPaused && svPaused->current.integer != 0;

    // CMSTATE_LIMBO (2) always uses the longer connection timeout. Otherwise,
    // CA_ACTIVE is the first state governed by cl_timeout rather than
    // cl_connectTimeout.
    const bool usesConnectTimeout = migrationState == 2 || connectionState <= CA_PRIMED;
    const float timeoutSeconds = usesConnectTimeout ? connectTimeoutSeconds : activeTimeoutSeconds;
    const bool timeoutExceeded =
        lastPacketTime > 0 && timeoutSeconds >= 0.0f && static_cast<float>(silenceMs) > timeoutSeconds * 1000.0f;
    const bool attemptsExceeded =
        lastPacketTime == 0 && connectionAttemptLimit >= 0 && connectPacketCount > connectionAttemptLimit;

    static int lastConnectionState[MAX_TRACKED_LOCAL_CLIENTS] = {-1, -1, -1, -1};
    static unsigned int lastLogTime[MAX_TRACKED_LOCAL_CLIENTS] = {};

    const unsigned int now = Sys_Milliseconds();
    const bool stateChanged = lastConnectionState[localClientNum] != connectionState;
    const bool periodicLogDue =
        connectionState > CA_LOGO && now - lastLogTime[localClientNum] >= TIMEOUT_LOG_INTERVAL_MS;

    if (stateChanged || periodicLogDue || timeoutExceeded || attemptsExceeded || timeoutCount > 0)
    {
        // Keep each variadic DbgPrint call within Xenon's seven register arguments.
        DbgPrint("[codxe][net-timeout] t=%u map=%s client=%d state=%s(%d) migration=%d server=%d\n", now,
                 GetUiMapName(), localClientNum, GetConnectionStateName(connectionState), connectionState,
                 migrationState, serverRunning ? 1 : 0);
        DbgPrint("[codxe][net-timeout]   paused=%d/%d realtime=%d lastPacket=%d silence=%dms timeoutCount=%d\n",
                 clientPaused ? 1 : 0, serverPaused ? 1 : 0, realtime, lastPacketTime, silenceMs, timeoutCount);
        DbgPrint("[codxe][net-timeout]   connectPackets=%d/%d limits(connect=%dms active=%dms) window=%s expired=%d\n",
                 connectPacketCount, connectionAttemptLimit, connectTimeoutMs, activeTimeoutMs,
                 usesConnectTimeout ? "connect" : "active", timeoutExceeded || attemptsExceeded ? 1 : 0);

        lastConnectionState[localClientNum] = connectionState;
        lastLogTime[localClientNum] = now;
    }

    return original(localClientNum);
}

void Party_Frame_Hook(void *party, unsigned int localControllerIndex, int msec, int a4, __int64 a5)
{
    auto original = Party_Frame_Detour.GetOriginal<Party_Frame_t>();
    auto partyBytes = static_cast<unsigned char *>(party);

    const bool preloadingMap = partyBytes[PARTY_PRELOADING_MAP_OFFSET] != 0;
    const int areWeHost = *reinterpret_cast<int *>(partyBytes + PARTY_ARE_WE_HOST_OFFSET);
    const int partyId = *reinterpret_cast<int *>(partyBytes + PARTY_ID_OFFSET);

    static void *trackedParty = nullptr;
    static int lastProgressBucket = -1;
    static unsigned int lastHeartbeat = 0;

    if (preloadingMap)
    {
        const float loadedFraction =
            DB_GetLoadedFraction(reinterpret_cast<int>(party), static_cast<int>(localControllerIndex), msec, a5);
        int progressPercent = static_cast<int>(loadedFraction * 100.0f);
        if (progressPercent < 0)
            progressPercent = 0;
        else if (progressPercent > 100)
            progressPercent = 100;

        const int progressBucket = progressPercent / 5;
        const unsigned int now = Sys_Milliseconds();
        const bool firstObservation = trackedParty != party;
        const bool heartbeatDue = now - lastHeartbeat >= 5000;

        if (firstObservation || progressBucket != lastProgressBucket || heartbeatDue)
        {
            const int loadedSize = *reinterpret_cast<volatile int *>(DB_LOADED_SIZE_ADDRESS);
            const int totalSize = *reinterpret_cast<volatile int *>(DB_TOTAL_SIZE_ADDRESS);
            const int loadedExternalBytes = *reinterpret_cast<volatile int *>(DB_LOADED_EXTERNAL_BYTES_ADDRESS);
            const int totalExternalBytes = *reinterpret_cast<volatile int *>(DB_TOTAL_EXTERNAL_BYTES_ADDRESS);

            DbgPrint("[codxe][preload] map=%s party=%p partyId=%d host=%d preloading=1 "
                     "loaded=%.3f (%d%%) threshold=%s\n",
                     GetUiMapName(), party, partyId, areWeHost, loadedFraction, progressPercent,
                     loadedFraction >= 0.25f ? "met" : "waiting");
            DbgPrint("[codxe][preload]   inline=%d/%d external=%d/%d\n", loadedSize, totalSize, loadedExternalBytes,
                     totalExternalBytes);

            trackedParty = party;
            lastProgressBucket = progressBucket;
            lastHeartbeat = now;
        }
    }

    original(party, localControllerIndex, msec, a4, a5);

    const bool preloadingMapAfter = partyBytes[PARTY_PRELOADING_MAP_OFFSET] != 0;
    if (preloadingMap && !preloadingMapAfter)
    {
        DbgPrint("[codxe][preload] map=%s Party_Frame cleared preloadingMap; vanilla threshold branch ran\n",
                 GetUiMapName());
        trackedParty = nullptr;
        lastProgressBucket = -1;
    }
}

void Cbuf_AddText_Hook(int localClientNum, const char *text)
{
    auto original = Cbuf_AddText_Detour.GetOriginal<Cbuf_AddText_t>();

    if (text && std::strncmp(text, "map ", 4) == 0)
        DbgPrint("[codxe][preload] Cbuf_AddText localClient=%d command=%s", localClientNum, text);

    original(localClientNum, text);
}

void SV_SpawnServer_Hook(char *server, int mapIsPreloaded, int savegame, bool cheats)
{
    auto original = SV_SpawnServer_Detour.GetOriginal<SV_SpawnServer_t>();

    DbgPrint("[codxe][preload] SV_SpawnServer begin: map=%s preloaded=%d savegame=%d cheats=%d\n",
             server ? server : "(null)", mapIsPreloaded, savegame, cheats);
    original(server, mapIsPreloaded, savegame, cheats);
    DbgPrint("[codxe][preload] SV_SpawnServer returned: map=%s\n", server ? server : "(null)");
}
} // namespace

fastfiles::fastfiles()
{
    Sys_CreateFile_Detour = Detour(Sys_CreateFile, Sys_CreateFile_Hook);
    Sys_CreateFile_Detour.Install();

    DB_LoadXAssets_Detour = Detour(DB_LoadXAssets, DB_LoadXAssets_Hook);
    DB_LoadXAssets_Detour.Install();

    Cbuf_AddText_Detour = Detour(Cbuf_AddText_Internal, Cbuf_AddText_Hook);
    Cbuf_AddText_Detour.Install();

    SV_SpawnServer_Detour = Detour(SV_SpawnServer, SV_SpawnServer_Hook);
    SV_SpawnServer_Detour.Install();

    Party_Frame_Detour = Detour(Party_Frame, Party_Frame_Hook);
    Party_Frame_Detour.Install();

    CL_CheckTimeout_Detour = Detour(CL_CheckTimeout, CL_CheckTimeout_Hook);
    CL_CheckTimeout_Detour.Install();
}

fastfiles::~fastfiles()
{
    CL_CheckTimeout_Detour.Remove();
    Party_Frame_Detour.Remove();
    SV_SpawnServer_Detour.Remove();
    Cbuf_AddText_Detour.Remove();
    DB_LoadXAssets_Detour.Remove();
    Sys_CreateFile_Detour.Remove();
}
} // namespace mp_tu6
} // namespace iw4
