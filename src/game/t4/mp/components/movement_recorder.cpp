#include "pch.h"
#include "movement_recorder.h"

namespace t4
{
namespace mp
{
namespace
{
enum RecorderMode
{
    RECORDER_IDLE,
    RECORDER_RECORDING,
    RECORDER_PLAYING,
};

struct RecordedCommand
{
    usercmd_s cmd;
    unsigned short worldAngles[3];
};

const size_t MAX_RECORDED_COMMANDS = 18000;
const float SHORT_TO_ANGLE = 360.0f / 65536.0f;
RecorderMode mode = RECORDER_IDLE;
std::vector<RecordedCommand> recording;
size_t playFrame = 0;
int initialCommandTime = 0;
int playbackBaseTime = 0;
bool playbackStarted = false;
Detour CL_FinishMove_Detour;
cmd_function_s commandStorage[7] = {};

bool IsInGame()
{
    return clientConnectionStates[0] == CA_ACTIVE && *clients && *cgArray;
}

void Message(const char *text)
{
    Com_Printf(0, "[codxe][T4][Recorder] %s\n", text);
    if (IsInGame())
        CG_GameMessage(0, text);
}

void StopPlayback()
{
    mode = RECORDER_IDLE;
    playFrame = 0;
    playbackStarted = false;
}

void StartRecord()
{
    if (!IsInGame())
        return Message("^1Join a game before recording.");
    if (mode == RECORDER_RECORDING)
        return Message("^3Already recording.");
    if (mode == RECORDER_PLAYING)
        return Message("^1Stop playback before recording.");

    recording.clear();
    initialCommandTime = 0;
    mode = RECORDER_RECORDING;
    Message("Recording ^2started");
}

void StopRecord()
{
    if (mode != RECORDER_RECORDING)
        return Message("^3Not currently recording.");

    mode = RECORDER_IDLE;
    char text[96];
    sprintf_s(text, "Recording ^1stopped^7: %u commands", static_cast<unsigned int>(recording.size()));
    Message(text);
}

void ToggleRecord()
{
    if (mode == RECORDER_RECORDING)
        StopRecord();
    else
        StartRecord();
}

void StartPlayback()
{
    if (!IsInGame())
        return Message("^1Join a game before playback.");
    if (mode == RECORDER_RECORDING)
        return Message("^1Stop recording before playback.");
    if (mode == RECORDER_PLAYING)
        return Message("^3Already playing.");
    if (recording.empty())
        return Message("^1No recording available.");

    playFrame = 0;
    playbackStarted = false;
    mode = RECORDER_PLAYING;
    Message("Playback ^2started");
}

void StopPlaybackCommand()
{
    if (mode != RECORDER_PLAYING)
        return Message("^3Not currently playing.");

    StopPlayback();
    Message("Playback ^1stopped");
}

void TogglePlayback()
{
    if (mode == RECORDER_PLAYING)
        StopPlaybackCommand();
    else
        StartPlayback();
}

void Status()
{
    char text[128];
    const char *state = mode == RECORDER_RECORDING ? "recording" : mode == RECORDER_PLAYING ? "playing" : "idle";
    sprintf_s(text, "%s: %u commands, replay %u", state, static_cast<unsigned int>(recording.size()),
              static_cast<unsigned int>(playFrame));
    Message(text);
}

void CaptureCommand(const usercmd_s *cmd, const clientActive_t *client, const playerState_s *ps)
{
    if (!recording.empty())
    {
        if (cmd->serverTime == recording.back().cmd.serverTime)
            return;
        if (cmd->serverTime < recording.back().cmd.serverTime)
        {
            MovementRecorder::OnCGInit(0);
            return Message("^1Recording cleared: command time restarted.");
        }
    }
    else
    {
        // Keep the first generated interval too, rather than treating the first command as time zero.
        const int previousTime = client->cmds[(client->cmdNumber - 1) & 0x7F].serverTime;
        const int interval = cmd->serverTime - previousTime;
        initialCommandTime = interval > 0 && interval <= 200 ? previousTime : cmd->serverTime;
    }

    RecordedCommand data = {};
    data.cmd = *cmd;
    for (int axis = 0; axis < 3; ++axis)
        data.worldAngles[axis] = static_cast<unsigned short>(cmd->angles[axis] + ANGLE2SHORT(ps->delta_angles[axis]));
    recording.push_back(data);

    if (recording.size() == MAX_RECORDED_COMMANDS)
        StopRecord();
}

void ReplayCommand(usercmd_s *cmd, clientActive_t *client, const playerState_s *ps)
{
    // Check live movement before changing any part of the fresh command.
    if (std::abs(static_cast<int>(cmd->forwardmove)) >= 75 || std::abs(static_cast<int>(cmd->rightmove)) >= 75)
        return StopPlaybackCommand();

    if (!playbackStarted)
    {
        const int previousTime = client->cmds[(client->cmdNumber - 1) & 0x7F].serverTime;
        const int interval = cmd->serverTime - previousTime;
        playbackBaseTime = interval > 0 && interval <= 200 ? previousTime : cmd->serverTime;
        playbackStarted = true;
    }

    const RecordedCommand &data = recording[playFrame];
    *cmd = data.cmd;
    cmd->serverTime = playbackBaseTime + (data.cmd.serverTime - initialCommandTime);
    for (int axis = 0; axis < 3; ++axis)
    {
        // Match the unsigned angle representation used by the network decoder.
        cmd->angles[axis] = static_cast<unsigned short>(data.worldAngles[axis] - ANGLE2SHORT(ps->delta_angles[axis]));
        client->viewangles[axis] = cmd->angles[axis] * SHORT_TO_ANGLE - client->cgameKickAngles[axis];
    }

    if (++playFrame == recording.size())
    {
        StopPlayback();
        Message("Playback ^2finished");
    }
}

int CL_FinishMove_Hook(int localClientNum, usercmd_s *cmd)
{
    const int result = CL_FinishMove_Detour.GetOriginal<decltype(CL_FinishMove)>()(localClientNum, cmd);
    // Console commands and the CJ binds control the primary local player's recording.
    if (localClientNum != 0 || !IsInGame() || mode == RECORDER_IDLE)
        return result;

    clientActive_t *client = &(*clients)[0];
    const playerState_s *ps = &(*cgArray)[0].predictedPlayerState;
    if (mode == RECORDER_RECORDING)
        CaptureCommand(cmd, client, ps);
    else
        ReplayCommand(cmd, client, ps);
    return result;
}
} // namespace

void MovementRecorder::OnCmdInit()
{
    Cmd_AddCommandInternal("startrecord", StartRecord, &commandStorage[0]);
    Cmd_AddCommandInternal("stoprecord", StopRecord, &commandStorage[1]);
    Cmd_AddCommandInternal("togglerecord", ToggleRecord, &commandStorage[2]);
    Cmd_AddCommandInternal("startplayback", StartPlayback, &commandStorage[3]);
    Cmd_AddCommandInternal("stopplayback", StopPlaybackCommand, &commandStorage[4]);
    Cmd_AddCommandInternal("toggleplayback", TogglePlayback, &commandStorage[5]);
    Cmd_AddCommandInternal("recorderstatus", Status, &commandStorage[6]);
}

void MovementRecorder::OnCGInit(int localClientNum)
{
    if (localClientNum != 0)
        return;

    StopPlayback();
    std::vector<RecordedCommand>().swap(recording);
    initialCommandTime = 0;
    playbackBaseTime = 0;
}

MovementRecorder::MovementRecorder()
{
    CL_FinishMove_Detour = Detour(CL_FinishMove, CL_FinishMove_Hook);
    CL_FinishMove_Detour.Install();
}

MovementRecorder::~MovementRecorder()
{
    CL_FinishMove_Detour.Remove();
}
} // namespace mp
} // namespace t4
