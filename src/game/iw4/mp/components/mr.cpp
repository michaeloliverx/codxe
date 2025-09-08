#include "mr.h"
#include "common.h"

#define ANGLE2SHORT(x) ((int)((x) * 65536 / 360) & 65535)
#define SHORT2ANGLE(x) ((x) * (360.0 / 65536))

namespace iw4
{
namespace mp
{
struct RecordedCmd
{
    int serverTime;
    int buttons;
    short angles[3];
    unsigned __int16 weapon;
    unsigned __int16 offHandIndex;
    char forwardmove;
    char rightmove;
};
size_t play_frame = 0;
bool is_recording = false;
bool is_playing = false;
int playback_start_time = 0;
int recording_start_time = 0;
std::vector<RecordedCmd> current_recording;

static cmd_function_s Cmd_Startrecord_VAR;
static cmd_function_s Cmd_Stoprecord_VAR;
static cmd_function_s Cmd_Togglerecord_VAR;
static cmd_function_s Cmd_Startplayback_VAR;
static cmd_function_s Cmd_Stopplayback_VAR;

void Cmd_Startrecord_f()
{
    if (is_recording)
    {
        CG_GameMessage(0, "^3Already recording");
        return;
    }

    is_recording = true;
    current_recording.clear();
    CG_GameMessage(0, "Recording ^2started");
}

void Cmd_Stoprecord_f()
{
    if (!is_recording)
    {
        CG_GameMessage(0, "^1Not currently recording.");
        return;
    }

    is_recording = false;
    CG_GameMessage(0, "Recording ^1stopped");
}

void Cmd_Togglerecord_f()
{
    if (is_recording)
    {
        Cmd_Stoprecord_f();
    }
    else
    {
        Cmd_Startrecord_f();
    }
}

void Cmd_Startplayback_f()
{
    if (is_recording)
    {
        CG_GameMessage(0, "^1Stop recording before starting playback.\n");
        return;
    }

    if (is_playing)
    {
        CG_GameMessage(0, "^3Already playing.\n");
        return;
    }

    if (current_recording.empty())
    {
        CG_GameMessage(0, "^1No recording available to play.\n");
        return;
    }

    play_frame = 0;
    is_playing = true;
    playback_start_time = 0; // Will be set on first UpdateCommand
    recording_start_time = current_recording[0].serverTime;
    CG_GameMessage(0, "Playback ^2started\n");
}

void Cmd_Stopplayback_f()
{
    if (!is_playing)
    {
        CG_GameMessage(0, "^1Not currently playing.\n");
        return;
    }

    play_frame = 0;
    is_playing = false;
    CG_GameMessage(0, "Playback ^1stopped\n");
}

bool IsPlayback()
{
    return is_playing;
}

void CaptureCommand(usercmd_s *const cmd)
{
    const playerState_s *ps = CG_GetPredictedPlayerState(0);

    RecordedCmd recorded_cmd;
    recorded_cmd.serverTime = cmd->serverTime;
    recorded_cmd.buttons = cmd->buttons;
    recorded_cmd.angles[PITCH] = static_cast<short>(cmd->angles[PITCH]) + ANGLE2SHORT(ps->delta_angles[PITCH]);
    recorded_cmd.angles[YAW] = static_cast<short>(cmd->angles[YAW]) + ANGLE2SHORT(ps->delta_angles[YAW]);
    recorded_cmd.angles[ROLL] = static_cast<short>(cmd->angles[ROLL]) + ANGLE2SHORT(ps->delta_angles[ROLL]);
    recorded_cmd.weapon = cmd->weapon;
    recorded_cmd.offHandIndex = cmd->offHandIndex;
    recorded_cmd.forwardmove = cmd->forwardmove;
    recorded_cmd.rightmove = cmd->rightmove;

    current_recording.push_back(recorded_cmd);
}

void UpdateCommand(usercmd_s *const cmd)
{
    if (current_recording.empty())
        return;

    if (play_frame >= current_recording.size())
    {
        Cmd_Stopplayback_f();
        return;
    }

    const playerState_s *ps = CG_GetPredictedPlayerState(0);
    auto ca = &(*clients)[0];
    const auto &data = current_recording[play_frame];

    // Initialize playback start time on first frame
    if (playback_start_time == 0)
    {
        playback_start_time = cmd->serverTime;
    }

    // Calculate the relative time offset from the start of the recording
    int recording_time_offset = data.serverTime - recording_start_time;

    // Apply this offset to the current playback time
    cmd->serverTime = playback_start_time + recording_time_offset;

    cmd->buttons = data.buttons;

    // Set the command angles to the recorded angles
    const auto pitch = data.angles[PITCH] - ANGLE2SHORT(ps->delta_angles[PITCH]);
    const auto yaw = data.angles[YAW] - ANGLE2SHORT(ps->delta_angles[YAW]);
    const auto roll = data.angles[ROLL] - ANGLE2SHORT(ps->delta_angles[ROLL]);

    cmd->angles[PITCH] = pitch;
    cmd->angles[YAW] = yaw;
    cmd->angles[ROLL] = roll;

    // Set client viewangles to match the recorded angles
    ca->viewangles[PITCH] = static_cast<float>(SHORT2ANGLE(pitch));
    ca->viewangles[YAW] = static_cast<float>(SHORT2ANGLE(yaw));
    ca->viewangles[ROLL] = static_cast<float>(SHORT2ANGLE(roll));

    cmd->weapon = data.weapon;
    cmd->offHandIndex = data.offHandIndex;
    cmd->forwardmove = data.forwardmove;
    cmd->rightmove = data.rightmove;

    play_frame++;
}

bool MovementRecorder::TAS_Enabled()
{
    const bool tas_enabled = (IsPlayback());
    return tas_enabled;
}

Detour CL_CreateNewCommands_Detour;

void CL_CreateNewCommands_Hook(int localClientNum)
{
    CL_CreateNewCommands_Detour.GetOriginal<decltype(CL_CreateNewCommands)>()(localClientNum);
    if (clientUIActives[localClientNum].connectionState == CA_ACTIVE)
    {
        auto ca = &(*clients)[localClientNum];
        auto cmd = &ca->cmds[ca->cmdNumber & 0x7F];

        if (is_recording)
        {
            CaptureCommand(cmd);
        }
        if (is_playing)
        {
            UpdateCommand(cmd);
        }
    }
}

MovementRecorder::MovementRecorder()
{
    CL_CreateNewCommands_Detour = Detour(CL_CreateNewCommands, CL_CreateNewCommands_Hook);
    CL_CreateNewCommands_Detour.Install();

    Cmd_AddCommandInternal("startrecord", Cmd_Startrecord_f, &Cmd_Startrecord_VAR);
    Cmd_AddCommandInternal("stoprecord", Cmd_Stoprecord_f, &Cmd_Stoprecord_VAR);
    Cmd_AddCommandInternal("togglerecord", Cmd_Togglerecord_f, &Cmd_Togglerecord_VAR);
    Cmd_AddCommandInternal("startplayback", Cmd_Startplayback_f, &Cmd_Startplayback_VAR);
    Cmd_AddCommandInternal("stopplayback", Cmd_Stopplayback_f, &Cmd_Stopplayback_VAR);
}

MovementRecorder::~MovementRecorder()
{
    CL_CreateNewCommands_Detour.Remove();
}
} // namespace mp
} // namespace iw4
