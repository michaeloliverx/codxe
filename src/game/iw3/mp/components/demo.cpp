#include "pch.h"
#include "demo.h"
#include "demo_stream.h"
#include "command.h"
#include "mods.h"
#include <errno.h>
#include <new>

namespace iw3
{
namespace mp
{
namespace
{
enum Mode
{
    IDLE,
    RECORDING,
    PLAYING
};

Mode mode = IDLE;
FILE *file = nullptr;
std::string demoName;
std::string lastStopReason;
iw3_demo::Header header;
iw3_demo::Index index;
iw3_demo::Clock clock;
gameState_t initialGameState;
snapshot_s scratchSnapshot;
playerState_s cameraFrom;
playerState_s cameraTo;
size_t cameraIndex = static_cast<size_t>(-1);
size_t snapshotIndex = static_cast<size_t>(-1);
clientActive_t *savedClient = nullptr;
clientConnection_t *savedConnection = nullptr;
int savedRealtime = 0;
bool rebuilding = false;
bool playbackError = false;
int lastMessage = -1;
int lastSnapshotTime = -1;
int lastCameraTime = -1;
int lastCommand = 0;
unsigned int snapshotCount = 0;
unsigned int cameraCount = 0;
double startTime = 0.0;
double endTime = 0.0;

dvar_s *demo_timescale = nullptr;
dvar_s *demo_captureFps = nullptr;
dvar_s *demo_smoothCamera = nullptr;
dvar_s *demo_holdAtEnd = nullptr;

Detour getSnapshotDetour;
Detour serverCommandDetour;
Detour frameDetour;
Detour drawFrameDetour;
Detour predictDetour;
Detour serverFrameDetour;
Detour writePacketDetour;
Detour parseMessageDetour;
Detour shutdownDetour;
Detour clearStateDetour;
Detour drawDisconnectDetour;

struct MovieDvar
{
    dvar_s *dvar;
    unsigned __int16 flags;
};
std::vector<MovieDvar> movieDvars;

void RememberMovieDvars()
{
    const char *names[] = {"cg_fov",
                           "cg_fovScale",
                           "cg_fovMin",
                           "cg_draw2D",
                           "cg_drawGun",
                           "r_filmUseTweaks",
                           "r_filmTweakEnable",
                           "r_filmTweakContrast",
                           "r_filmTweakBrightness",
                           "r_filmTweakDesaturation",
                           "r_filmTweakInvert",
                           "r_filmTweakLightTint",
                           "r_filmTweakDarkTint"};
    for (size_t i = 0; i < ARRAYSIZE(names); ++i)
    {
        dvar_s *dvar = Dvar_FindMalleableVar(names[i]);
        if (dvar)
        {
            MovieDvar saved = {dvar, dvar->flags};
            movieDvars.push_back(saved);
        }
    }
}

void UnlockMovieDvars()
{
    for (size_t i = 0; i < movieDvars.size(); ++i)
        movieDvars[i].dvar->flags &= ~DVAR_CHEAT;
}

int Argc()
{
    return cmd_args->nesting >= 0 && cmd_args->nesting < 8 ? cmd_args->argc[cmd_args->nesting] : 0;
}

const char *Argv(int arg)
{
    return arg >= 0 && arg < Argc() ? cmd_args->argv[cmd_args->nesting][arg] : "";
}

iw3_demo::Header Layout()
{
    iw3_demo::Header result = {};
    result.gameStateSize = sizeof(gameState_t);
    result.playerStateSize = sizeof(playerState_s);
    result.entityStateSize = sizeof(entityState_s);
    result.clientStateSize = sizeof(clientState_s);
    result.snapshotPrefixSize = offsetof(snapshot_s, entities);
    return result;
}

bool ValidName(const char *name)
{
    if (!name || !*name || strlen(name) > 63)
        return false;
    for (const char *c = name; *c; ++c)
        if (!((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') || (*c >= '0' && *c <= '9') || *c == '_' ||
              *c == '-'))
            return false;
    return true;
}

std::string DemoPath(const char *name)
{
    return filesystem::JoinPath(Config::GetDataRoot(), (std::string("demos/") + name + ".xedemo").c_str());
}

bool ActiveClient()
{
    if (!*clients || !*cgArray || !*clientConnections || !*cgsArray ||
        clientUIActives[0].connectionState != CA_ACTIVE || !clientUIActives[0].cgameInitialized)
        return false;
    for (int i = 1; i < 4; ++i)
        if (clientUIActives[i].active && clientUIActives[i].connectionState >= CA_CONNECTED)
            return false;
    return true;
}

bool LocalPlaybackSession()
{
    if (!ActiveClient() || !Dvar_GetBool("sv_running") || !svs->clients)
        return false;
    const int count = Dvar_GetInt("sv_maxclients");
    if (count < 1 || count > MAX_CLIENTS)
        return false;
    for (int i = 0; i < count; ++i)
    {
        const client_t &client = svs->clients[i];
        if (client.header.state >= CS_CONNECTED && client.header.netchan.remoteAddress.type != NA_LOOPBACK &&
            client.header.netchan.remoteAddress.type != NA_BOT)
            return false;
    }
    return true;
}

bool ValidGameState(const gameState_t &state)
{
    if (state.dataCount < 1 || state.dataCount > static_cast<int>(sizeof(state.stringData)) || state.stringData[0])
        return false;
    for (int i = 0; i < MAX_CONFIGSTRINGS; ++i)
    {
        const int offset = state.stringOffsets[i];
        if (offset < 0 || offset >= state.dataCount || !memchr(state.stringData + offset, 0, state.dataCount - offset))
            return false;
    }
    return true;
}

void StopRecord(const char *reason)
{
    if (mode != RECORDING)
        return;
    bool ok = iw3_demo::WriteRecord(file, iw3_demo::END, 0, lastSnapshotTime < 0 ? 0 : lastSnapshotTime);
    if (fflush(file) != 0)
        ok = false;
    if (fclose(file) != 0)
        ok = false;
    file = nullptr;
    mode = IDLE;
    lastStopReason = reason;
    DbgPrint("[codxe][IW3 demo] %s: %s, snapshots=%u cameras=%u writeOk=%d\n", demoName.c_str(), reason, snapshotCount,
             cameraCount, ok ? 1 : 0);
    Com_Printf(CON_CHANNEL_CONSOLEONLY, "Demo %s: %s (%u snapshots, %u camera samples).%s\n", demoName.c_str(), reason,
               snapshotCount, cameraCount, ok ? "" : " Write failed; complete records may be recoverable.");
}

bool CaptureSnapshot(int message, const snapshot_s &snapshot)
{
    if (mode != RECORDING || message <= lastMessage)
        return true;
    if (snapshot.serverTime < 0 || snapshot.serverTime <= lastSnapshotTime || snapshot.numEntities < 0 ||
        snapshot.numEntities > 512 || snapshot.numClients < 0 || snapshot.numClients > 24)
    {
        StopRecord("snapshot clock changed; start a new recording");
        return false;
    }
    const clientConnection_t &connection = (*clientConnections)[0];
    const int sequence = snapshot.serverCommandSequence;
    if (sequence < lastCommand || sequence > connection.serverCommandSequence ||
        lastCommand < connection.serverCommandSequence - 128)
    {
        StopRecord("server command history was lost");
        return false;
    }
    while (lastCommand < sequence)
    {
        const char *text = connection.serverCommands[(lastCommand + 1) & 127];
        const char *end = static_cast<const char *>(memchr(text, 0, 1024));
        if (!end ||
            !iw3_demo::WriteRecord(file, iw3_demo::COMMAND, static_cast<uint32_t>(end - text + 1), snapshot.serverTime,
                                   lastCommand + 1) ||
            !iw3_demo::Write(file, text, end - text + 1))
        {
            StopRecord("could not write server command");
            return false;
        }
        ++lastCommand;
    }
    const uint32_t size = header.snapshotPrefixSize + snapshot.numEntities * sizeof(entityState_s) +
                          snapshot.numClients * sizeof(clientState_s) + sizeof(int);
    if (!iw3_demo::WriteRecord(file, iw3_demo::SNAPSHOT, size, snapshot.serverTime) ||
        !iw3_demo::Write(file, &snapshot, header.snapshotPrefixSize) ||
        !iw3_demo::Write(file, snapshot.entities, snapshot.numEntities * sizeof(entityState_s)) ||
        !iw3_demo::Write(file, snapshot.clients, snapshot.numClients * sizeof(clientState_s)) ||
        !iw3_demo::Write(file, &sequence, sizeof(sequence)))
    {
        StopRecord("could not write snapshot (disk full or 2 GB file limit)");
        return false;
    }
    lastMessage = message;
    lastSnapshotTime = snapshot.serverTime;
    ++snapshotCount;
    // Bound data loss on a crash without flushing every render frame.
    if ((snapshotCount % 20) == 0 && fflush(file) != 0)
    {
        StopRecord("could not flush recording");
        return false;
    }
    return true;
}

bool ReadSnapshot(size_t number, snapshot_s *snapshot)
{
    if (number >= index.snapshots.size())
        return false;
    const iw3_demo::Record &record = index.snapshots[number];
    memset(snapshot, 0, sizeof(*snapshot));
    if (fseek(file, record.offset, SEEK_SET) != 0 || !iw3_demo::Read(file, snapshot, header.snapshotPrefixSize) ||
        snapshot->numEntities < 0 || snapshot->numEntities > 512 || snapshot->numClients < 0 ||
        snapshot->numClients > 24 || snapshot->serverTime != static_cast<int>(record.time))
        return false;
    const uint32_t size = header.snapshotPrefixSize + snapshot->numEntities * sizeof(entityState_s) +
                          snapshot->numClients * sizeof(clientState_s) + sizeof(int);
    if (size != record.size ||
        !iw3_demo::Read(file, snapshot->entities, snapshot->numEntities * sizeof(entityState_s)) ||
        !iw3_demo::Read(file, snapshot->clients, snapshot->numClients * sizeof(clientState_s)) ||
        !iw3_demo::Read(file, &snapshot->serverCommandSequence, sizeof(int)) ||
        snapshot->serverCommandSequence < static_cast<int>(header.commandSequence) ||
        static_cast<size_t>(snapshot->serverCommandSequence - header.commandSequence) > index.commands.size() ||
        snapshot->ps.clientNum < 0 || snapshot->ps.clientNum >= MAX_CLIENTS)
        return false;
    for (int i = 0; i < snapshot->numEntities; ++i)
        if (snapshot->entities[i].number < 0 || snapshot->entities[i].number >= 1024)
            return false;
    for (int i = 0; i < snapshot->numClients; ++i)
        if (snapshot->clients[i].clientIndex < 0 || snapshot->clients[i].clientIndex >= MAX_CLIENTS)
            return false;
    return true;
}

bool ReadCamera(size_t number, playerState_s &state)
{
    if (number >= index.cameras.size() || fseek(file, index.cameras[number].offset, SEEK_SET) != 0 ||
        !iw3_demo::Read(file, &state, sizeof(state)) || state.clientNum < 0 || state.clientNum >= MAX_CLIENTS)
        return false;
    for (int axis = 0; axis < 3; ++axis)
        if (!_finite(state.origin[axis]) || !_finite(state.velocity[axis]) || !_finite(state.viewangles[axis]) ||
            fabsf(state.viewangles[axis]) > 360000.0f)
            return false;
    return _finite(state.viewHeightCurrent) && _finite(state.fWeaponPosFrac) && _finite(state.leanf);
}

void ClosePlayback()
{
    for (size_t i = 0; i < movieDvars.size(); ++i)
        movieDvars[i].dvar->flags = movieDvars[i].flags;
    movieDvars.clear();
    if (file)
        fclose(file);
    file = nullptr;
    mode = IDLE;
    index = iw3_demo::Index();
    delete savedClient;
    delete savedConnection;
    savedClient = nullptr;
    savedConnection = nullptr;
    playbackError = false;
}

void StopPlayback(bool resume)
{
    if (mode != PLAYING)
        return;
    rebuilding = true;
    if (resume)
        CG_Shutdown(0);
    // The server was suspended, so its saved snapshot/command rings are still valid.
    memcpy(&(*clients)[0], savedClient, sizeof(*savedClient));
    memcpy(&(*clientConnections)[0], savedConnection, sizeof(*savedConnection));
    clientActive_t &client = (*clients)[0];
    clientConnection_t &connection = (*clientConnections)[0];
    client.serverTimeDelta -= *cl_realtime - savedRealtime;
    connection.lastPacketTime = *cl_realtime;
    connection.lastPacketSentTime = *cl_realtime;
    ClosePlayback();
    if (resume)
        CG_Init(0, client.snap.messageNum - 1, connection.lastExecutedServerCommand, connection.clientNum);
    rebuilding = false;
    Com_Printf(CON_CHANNEL_CONSOLEONLY, "Demo stopped.\n");
}

void PublishTime()
{
    size_t next = iw3_demo::Find(index.snapshots, clock.time);
    if (next + 1 < index.snapshots.size())
        ++next;
    if (snapshotIndex != next && !ReadSnapshot(next, &scratchSnapshot))
    {
        playbackError = true;
        return;
    }
    snapshotIndex = next;
    clientActive_t &client = (*clients)[0];
    client.serverTime = static_cast<int>(clock.time);
    client.oldServerTime = client.serverTime;
    client.oldFrameServerTime = client.serverTime;
    client.newSnapshots = 0;
    client.timeoutcount = 0;
    (*clientConnections)[0].lastPacketTime = *cl_realtime;
    client.snap.valid = 1;
    client.snap.messageNum = static_cast<int>(next + 1);
    client.snap.serverTime = scratchSnapshot.serverTime;
    client.snap.snapFlags = scratchSnapshot.snapFlags;
    client.snap.ps = scratchSnapshot.ps;
    client.snap.serverCommandNum = scratchSnapshot.serverCommandSequence;
}

void ResetPlayback(double time)
{
    rebuilding = true;
    CG_Shutdown(0);
    (*clients)[0].gameState = initialGameState;
    clientConnection_t &connection = (*clientConnections)[0];
    connection.clientNum = header.clientNum;
    connection.serverMessageSequence = 0;
    connection.serverCommandSequence = header.commandSequence;
    connection.lastExecutedServerCommand = header.commandSequence;
    memset(connection.serverCommands, 0, sizeof(connection.serverCommands));
    cameraIndex = static_cast<size_t>(-1);
    snapshotIndex = static_cast<size_t>(-1);
    clock.Reset(time, Sys_Milliseconds());
    PublishTime();
    CG_Init(0, 0, header.commandSequence, header.clientNum);
    (*cgArray)[0].demoType = DEMO_TYPE_CLIENT;
    UnlockMovieDvars();
    rebuilding = false;
    // Loading time must not advance the first playback frame.
    clock.Reset(time, Sys_Milliseconds());
}

void Record_f()
{
    if (mode != IDLE || !ActiveClient() || Argc() > 2)
    {
        Com_Printf(CON_CHANNEL_CONSOLEONLY,
                   "record [name]: enter a level with one local player; stop any current demo.\n");
        return;
    }
    std::string name = Argv(1);
    if (name.empty())
    {
        char automatic[32];
        for (int i = 0; i < 10000; ++i)
        {
            sprintf_s(automatic, "demo%04d", i);
            if (!filesystem::FileExists(DemoPath(automatic).c_str()))
            {
                name = automatic;
                break;
            }
        }
    }
    if (!ValidName(name.c_str()))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Demo names use 1-63 letters, digits, underscores or hyphens.\n");
        return;
    }
    const std::string path = DemoPath(name.c_str());
    header = Layout();
    header.clientNum = (*clientConnections)[0].clientNum;
    header.commandSequence = (*clientConnections)[0].lastExecutedServerCommand;
    strcpy_s(header.map, Dvar_GetString("mapname"));
    strcpy_s(header.mod, ModList::GetActiveName().c_str());
    initialGameState = (*clients)[0].gameState;
    if (!ValidGameState(initialGameState) || !ValidName(header.map))
        return;
    const int message = (*clients)[0].snap.messageNum;
    if (!getSnapshotDetour.GetOriginal<decltype(CL_GetSnapshot)>()(0, message, &scratchSnapshot))
    {
        Com_PrintError(CON_CHANNEL_ERROR, "No initial snapshot; existing recording was not overwritten.\n");
        return;
    }
    filesystem::CreateParentDirectories(path.c_str());
    file = fopen(path.c_str(), "wb");
    if (!file)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Could not create demo: %s (errno %d)\n", path.c_str(), errno);
        return;
    }
    if (!iw3_demo::WriteHeader(file, header) || !iw3_demo::Write(file, &initialGameState, sizeof(initialGameState)))
    {
        fclose(file);
        file = nullptr;
        Com_PrintError(CON_CHANNEL_ERROR, "Could not write demo header.\n");
        return;
    }
    demoName = name;
    lastMessage = lastSnapshotTime = lastCameraTime = -1;
    snapshotCount = cameraCount = 0;
    lastCommand = header.commandSequence;
    mode = RECORDING;
    lastStopReason.clear();
    if (!CaptureSnapshot(message, scratchSnapshot))
    {
        StopRecord("no initial snapshot");
        return;
    }
    Com_Printf(CON_CHANNEL_CONSOLEONLY, "Recording %s\n", path.c_str());
    DbgPrint("[codxe][IW3 demo] Recording %s\n", path.c_str());
}

void StopRecord_f()
{
    if (mode == RECORDING)
        StopRecord("saved");
    else
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "No demo recording is active.\n");
}

void Play_f()
{
    if (mode != IDLE || Argc() != 2 || !ValidName(Argv(1)))
    {
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "demo <name>: stop any current recording/playback first.\n");
        return;
    }
    if (!LocalPlaybackSession())
    {
        Com_PrintError(CON_CHANNEL_ERROR,
                       "Load the demo's map in a local host session with no remote players first.\n");
        return;
    }
    demoName = Argv(1);
    file = fopen(DemoPath(demoName.c_str()).c_str(), "rb");
    if (!file)
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Could not open demo %s.\n", demoName.c_str());
        return;
    }
    if (!iw3_demo::ReadHeader(file, header, Layout()) ||
        !iw3_demo::Read(file, &initialGameState, sizeof(initialGameState)) || !ValidGameState(initialGameState) ||
        !iw3_demo::Scan(file, header, index))
    {
        ClosePlayback();
        Com_PrintError(CON_CHANNEL_ERROR, "Invalid/incompatible demo, or fewer than two snapshots/camera samples.\n");
        return;
    }
    if (strcmp(header.map, Dvar_GetString("mapname")) || header.mod != ModList::GetActiveName())
    {
        Com_PrintError(CON_CHANNEL_ERROR, "Demo requires map %s and mod '%s'.\n", header.map, header.mod);
        ClosePlayback();
        return;
    }
    startTime =
        static_cast<double>(index.snapshots.front().time > index.cameras.front().time ? index.snapshots.front().time
                                                                                      : index.cameras.front().time);
    endTime = static_cast<double>(index.snapshots.back().time < index.cameras.back().time ? index.snapshots.back().time
                                                                                          : index.cameras.back().time);
    // Validate all snapshot counts/indices before changing the live cgame.
    bool valid = endTime > startTime;
    for (size_t i = 0; valid && i < index.snapshots.size(); ++i)
        valid = ReadSnapshot(i, &scratchSnapshot);
    if (!valid)
    {
        ClosePlayback();
        Com_PrintError(CON_CHANNEL_ERROR, "Demo has invalid snapshots or no overlapping camera timeline.\n");
        return;
    }
    savedClient = new (std::nothrow) clientActive_t;
    savedConnection = new (std::nothrow) clientConnection_t;
    if (!savedClient || !savedConnection)
    {
        ClosePlayback();
        Com_PrintError(CON_CHANNEL_ERROR, "Not enough memory for demo playback.\n");
        return;
    }
    memcpy(savedClient, &(*clients)[0], sizeof(*savedClient));
    memcpy(savedConnection, &(*clientConnections)[0], sizeof(*savedConnection));
    savedRealtime = *cl_realtime;
    RememberMovieDvars();
    mode = PLAYING;
    clock.paused = false;
    ResetPlayback(startTime);
    DbgPrint("[codxe][IW3 demo] Playing %s, start=%.3f end=%.3f\n", demoName.c_str(), startTime, endTime);
    Com_Printf(CON_CHANNEL_CONSOLEONLY, "Playing %s (%.3f seconds).%s\n", demoName.c_str(),
               (endTime - startTime) / 1000.0, index.complete ? "" : " Recovered incomplete recording.");
}

void StopDemo_f()
{
    if (mode == RECORDING)
        StopRecord("saved");
    else if (mode == PLAYING)
        StopPlayback(true);
    else
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "No demo is active.\n");
}

void Pause_f()
{
    if (mode == PLAYING)
    {
        clock.paused = !clock.paused;
        clock.Reset(clock.time, Sys_Milliseconds());
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "Demo %s at %.3f seconds.\n", clock.paused ? "paused" : "resumed",
                   (clock.time - startTime) / 1000.0);
    }
}

void Seek_f()
{
    if (mode != PLAYING)
        return;
    char *end;
    const double seconds = strtod(Argv(1), &end);
    if (Argc() != 2 || end == Argv(1) || *end || !_finite(seconds) || seconds < 0.0)
    {
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "demoseek <seconds from start>\n");
        return;
    }
    double time = startTime + seconds * 1000.0;
    if (time > endTime)
        time = endTime;
    ResetPlayback(time);
}

void Restart_f()
{
    if (mode == PLAYING)
        ResetPlayback(startTime);
}

void Status_f()
{
    if (mode == PLAYING)
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "Demo %s: %.3f / %.3f seconds, %.4fx%s, %u snapshots.\n", demoName.c_str(),
                   (clock.time - startTime) / 1000.0, (endTime - startTime) / 1000.0, demo_timescale->current.value,
                   clock.paused ? " (paused)" : "", static_cast<unsigned int>(index.snapshots.size()));
    else
    {
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "Demo %s%s.\n", mode == RECORDING ? "recording " : "idle",
                   mode == RECORDING ? demoName.c_str() : "");
        if (mode == IDLE && !lastStopReason.empty())
            Com_Printf(CON_CHANNEL_CONSOLEONLY, "Last recording %s: %s.\n", demoName.c_str(), lastStopReason.c_str());
    }
}

int GetSnapshot_Hook(int localClientNum, int number, snapshot_s *snapshot)
{
    if (mode == PLAYING && !localClientNum)
    {
        if (number <= 0 || !ReadSnapshot(number - 1, snapshot))
        {
            playbackError = true;
            return 0;
        }
        return 1;
    }
    const int result = getSnapshotDetour.GetOriginal<decltype(CL_GetSnapshot)>()(localClientNum, number, snapshot);
    if (result && !localClientNum && mode == RECORDING)
        CaptureSnapshot(number, *snapshot);
    return result;
}

int ServerCommand_Hook(int localClientNum, int sequence)
{
    if (mode == PLAYING && !localClientNum)
    {
        const int number = sequence - static_cast<int>(header.commandSequence) - 1;
        if (number < 0 || static_cast<size_t>(number) >= index.commands.size())
        {
            playbackError = true;
            return 0;
        }
        const iw3_demo::Record &record = index.commands[number];
        clientConnection_t &connection = (*clientConnections)[0];
        char *text = connection.serverCommands[sequence & 127];
        if (fseek(file, record.offset, SEEK_SET) != 0 || !iw3_demo::Read(file, text, record.size) ||
            text[record.size - 1])
        {
            playbackError = true;
            return 0;
        }
        // Feed one command at a time, including when seeking past the native 128-command ring.
        connection.serverCommandSequence = sequence;
    }
    return serverCommandDetour.GetOriginal<decltype(CL_CGameNeedsServerCommand)>()(localClientNum, sequence);
}

void Frame_Hook(int localClientNum, int msec)
{
    if (mode == PLAYING && !localClientNum)
    {
        if (!ActiveClient() || !Dvar_GetBool("sv_running"))
        {
            StopPlayback(false);
            return;
        }
        if (playbackError)
        {
            Com_PrintError(CON_CHANNEL_ERROR, "Demo playback stopped: state/read failure.\n");
            StopPlayback(true);
            return;
        }
        if (clock.finished && !demo_holdAtEnd->current.enabled)
        {
            Com_Printf(CON_CHANNEL_CONSOLEONLY, "Demo finished.\n");
            StopPlayback(true);
            return;
        }
        // Advance at the render boundary so fixed stepping counts rendered frames only.
        return;
    }
    frameDetour.GetOriginal<decltype(CL_Frame)>()(localClientNum, msec);
}

int DrawFrame_Hook(int localClientNum, int serverTime, DemoType demoType, CubemapShot shot, int size, int renderScreen)
{
    if (mode == PLAYING && !localClientNum)
    {
        const int captureFps = demo_captureFps->current.integer;
        clock.Tick(Sys_Milliseconds(), demo_timescale->current.value, endTime,
                   captureFps > 0 ? 1000.0 / captureFps : 0.0);
        PublishTime();
        if (playbackError)
            return 0;
        serverTime = static_cast<int>(clock.time);
        demoType = DEMO_TYPE_CLIENT;
    }
    return drawFrameDetour.GetOriginal<decltype(CG_DrawActiveFrame)>()(localClientNum, serverTime, demoType, shot, size,
                                                                       renderScreen);
}

void Predict_Hook(int localClientNum)
{
    predictDetour.GetOriginal<decltype(CG_PredictPlayerState)>()(localClientNum);
    if (localClientNum)
        return;
    cg_s &cg = (*cgArray)[0];
    if (mode == RECORDING && cg.time > lastCameraTime)
    {
        if (!iw3_demo::WriteRecord(file, iw3_demo::CAMERA, sizeof(playerState_s), cg.time) ||
            !iw3_demo::Write(file, &cg.predictedPlayerState, sizeof(playerState_s)))
            StopRecord("could not write camera sample");
        else
        {
            lastCameraTime = cg.time;
            ++cameraCount;
        }
    }
    else if (mode == PLAYING && demo_smoothCamera->current.enabled)
    {
        const size_t number = iw3_demo::Find(index.cameras, clock.time);
        const size_t next = number + 1 < index.cameras.size() ? number + 1 : number;
        if (number != cameraIndex)
        {
            if (!ReadCamera(number, cameraFrom) || !ReadCamera(next, cameraTo))
            {
                playbackError = true;
                return;
            }
            cameraIndex = number;
        }
        cg.predictedPlayerState = cameraFrom;
        playerState_s &state = cg.predictedPlayerState;
        const int gap = index.cameras[next].time - index.cameras[number].time;
        if (gap > 0 && gap <= 250 && cameraFrom.clientNum == cameraTo.clientNum &&
            cameraFrom.pm_type == cameraTo.pm_type && !((cameraFrom.eFlags ^ cameraTo.eFlags) & 2))
        {
            const float fraction = static_cast<float>((clock.time - index.cameras[number].time) / gap);
            for (int axis = 0; axis < 3; ++axis)
            {
                state.origin[axis] += fraction * (cameraTo.origin[axis] - cameraFrom.origin[axis]);
                state.velocity[axis] += fraction * (cameraTo.velocity[axis] - cameraFrom.velocity[axis]);
                state.viewangles[axis] =
                    iw3_demo::LerpAngle(cameraFrom.viewangles[axis], cameraTo.viewangles[axis], fraction);
            }
            state.viewHeightCurrent += fraction * (cameraTo.viewHeightCurrent - cameraFrom.viewHeightCurrent);
            state.fWeaponPosFrac += fraction * (cameraTo.fWeaponPosFrac - cameraFrom.fWeaponPosFrac);
            state.leanf += fraction * (cameraTo.leanf - cameraFrom.leanf);
            state.bobCycle =
                (cameraFrom.bobCycle + static_cast<int>(fraction * ((cameraTo.bobCycle - cameraFrom.bobCycle) & 255))) &
                255;
        }
        cg.predictedErrorTime = 0;
        memset(cg.predictedError, 0, sizeof(cg.predictedError));
        // The outer prediction function updated the local pose before this camera override.
        if (cg_entitiesArray[0])
            memcpy(cg_entitiesArray[0][state.clientNum].pose.origin, state.origin, sizeof(state.origin));
    }
}

int ServerFrame_Hook(int msec)
{
    return mode == PLAYING ? msec : serverFrameDetour.GetOriginal<decltype(SV_Frame)>()(msec);
}

void WritePacket_Hook(int localClientNum)
{
    if (mode != PLAYING)
        writePacketDetour.GetOriginal<decltype(CL_WritePacket)>()(localClientNum);
}

void ParseMessage_Hook(int localClientNum, void *message)
{
    if (mode != PLAYING)
        parseMessageDetour.GetOriginal<decltype(CL_ParseServerMessage)>()(localClientNum, message);
}

void Shutdown_Hook(int localClientNum)
{
    if (!localClientNum && !rebuilding)
    {
        StopRecord("level ended");
        StopPlayback(false);
    }
    shutdownDetour.GetOriginal<decltype(CG_Shutdown)>()(localClientNum);
}

void ClearState_Hook(int localClientNum)
{
    if (!localClientNum && !rebuilding)
    {
        StopRecord("client state cleared");
        StopPlayback(false);
    }
    clearStateDetour.GetOriginal<decltype(CL_ClearState)>()(localClientNum);
}

void DrawDisconnect_Hook(int localClientNum)
{
    // Recorded snapshots do not acknowledge the live input command ring.
    if (mode != PLAYING)
        drawDisconnectDetour.GetOriginal<decltype(CG_DrawDisconnect)>()(localClientNum);
}
} // namespace

void Demo::OnDvarInit()
{
    demo_timescale = Dvar_RegisterFloat("demo_timescale", 1.0f, 0.001f, 16.0f, 0, "Demo playback speed");
    demo_captureFps = Dvar_RegisterInt("demo_captureFps", 0, 0, 1000, 0,
                                       "Advance demo by 1000/fps milliseconds per rendered frame; 0 uses real time");
    demo_smoothCamera =
        Dvar_RegisterBool("demo_smoothCamera", true, 0, "Use recorded render-frame player states in demos");
    demo_holdAtEnd =
        Dvar_RegisterBool("demo_holdAtEnd", false, 0, "Hold the final demo frame instead of returning to play");
}

Demo::Demo()
{
    command::add("record", Record_f);
    command::add("demorecord", Record_f);
    command::add("demostoprecord", StopRecord_f);
    command::add("demo", Play_f);
    command::add("stopdemo", StopDemo_f);
    command::add("demopause", Pause_f);
    command::add("demoseek", Seek_f);
    command::add("demorestart", Restart_f);
    command::add("demostatus", Status_f);

    getSnapshotDetour = Detour(CL_GetSnapshot, GetSnapshot_Hook);
    serverCommandDetour = Detour(CL_CGameNeedsServerCommand, ServerCommand_Hook);
    frameDetour = Detour(CL_Frame, Frame_Hook);
    drawFrameDetour = Detour(CG_DrawActiveFrame, DrawFrame_Hook);
    predictDetour = Detour(CG_PredictPlayerState, Predict_Hook);
    serverFrameDetour = Detour(SV_Frame, ServerFrame_Hook);
    writePacketDetour = Detour(CL_WritePacket, WritePacket_Hook);
    parseMessageDetour = Detour(CL_ParseServerMessage, ParseMessage_Hook);
    shutdownDetour = Detour(CG_Shutdown, Shutdown_Hook);
    clearStateDetour = Detour(CL_ClearState, ClearState_Hook);
    drawDisconnectDetour = Detour(CG_DrawDisconnect, DrawDisconnect_Hook);
    getSnapshotDetour.Install();
    serverCommandDetour.Install();
    frameDetour.Install();
    drawFrameDetour.Install();
    predictDetour.Install();
    serverFrameDetour.Install();
    writePacketDetour.Install();
    parseMessageDetour.Install();
    shutdownDetour.Install();
    clearStateDetour.Install();
    drawDisconnectDetour.Install();
}

Demo::~Demo()
{
    StopRecord("plugin unloaded");
    // Game memory can already be going away during title unload.
    movieDvars.clear();
    ClosePlayback();
    drawDisconnectDetour.Remove();
    clearStateDetour.Remove();
    shutdownDetour.Remove();
    parseMessageDetour.Remove();
    writePacketDetour.Remove();
    serverFrameDetour.Remove();
    predictDetour.Remove();
    drawFrameDetour.Remove();
    frameDetour.Remove();
    serverCommandDetour.Remove();
    getSnapshotDetour.Remove();
}
} // namespace mp
} // namespace iw3
