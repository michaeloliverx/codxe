#include "mr.h"
#include "common.h"
#include "events.h"

#define ANGLE2SHORT(x) ((int)((x) * 65536 / 360) & 65535)
#define SHORT2ANGLE(x) ((x) * (360.0 / 65536))

namespace iw4
{
namespace mp
{

// ============================================================================
// Data Structures and State
// ============================================================================

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

struct RecordingSession
{
    float startOrigin[3];
    float startAngles[3];
    std::vector<RecordedCmd> commands;
};

// Global state variables
size_t play_frame = 0;
bool is_recording = false;
bool is_playing = false;
int playback_start_time = 0;
int recording_start_time = 0;
RecordingSession current_session;

// Command function variables
static cmd_function_s Cmd_Startrecord_VAR;
static cmd_function_s Cmd_Stoprecord_VAR;
static cmd_function_s Cmd_Togglerecord_VAR;
static cmd_function_s Cmd_Startplayback_VAR;
static cmd_function_s Cmd_Stopplayback_VAR;
static cmd_function_s Cmd_Saverecord_VAR;
static cmd_function_s Cmd_Loadrecord_VAR;

static auto Cmd_Argv = reinterpret_cast<const char *(*)(int argIndex)>(0x821235F8);
static auto TeleportPlayer =
    reinterpret_cast<void (*)(gentity_s *player, const float *origin, const float *angles)>(0x82234408);

// ============================================================================
// Serialization Functions
// ============================================================================

bool SerializeRecordingSession(const RecordingSession &session, std::vector<char> &buffer)
{
    buffer.clear();

    // Calculate total size needed
    size_t total_size = sizeof(session.startOrigin) + sizeof(session.startAngles) + sizeof(size_t) +
                        (session.commands.size() * sizeof(RecordedCmd));

    buffer.reserve(total_size);

    // Write start origin
    const char *origin_ptr = reinterpret_cast<const char *>(session.startOrigin);
    buffer.insert(buffer.end(), origin_ptr, origin_ptr + sizeof(session.startOrigin));

    // Write start angles
    const char *angles_ptr = reinterpret_cast<const char *>(session.startAngles);
    buffer.insert(buffer.end(), angles_ptr, angles_ptr + sizeof(session.startAngles));

    // Write command count
    size_t command_count = session.commands.size();
    const char *count_ptr = reinterpret_cast<const char *>(&command_count);
    buffer.insert(buffer.end(), count_ptr, count_ptr + sizeof(size_t));

    // Write commands
    for (auto it = session.commands.begin(); it != session.commands.end(); ++it)
    {
        const char *cmd_ptr = reinterpret_cast<const char *>(&(*it));
        buffer.insert(buffer.end(), cmd_ptr, cmd_ptr + sizeof(RecordedCmd));
    }

    return true;
}

bool DeserializeRecordingSession(const std::vector<char> &buffer, RecordingSession &session)
{
    if (buffer.size() < sizeof(session.startOrigin) + sizeof(session.startAngles) + sizeof(size_t))
    {
        return false;
    }

    size_t offset = 0;

    // Read start origin
    memcpy(session.startOrigin, &buffer[offset], sizeof(session.startOrigin));
    offset += sizeof(session.startOrigin);

    // Read start angles
    memcpy(session.startAngles, &buffer[offset], sizeof(session.startAngles));
    offset += sizeof(session.startAngles);

    // Read command count
    size_t command_count;
    memcpy(&command_count, &buffer[offset], sizeof(size_t));
    offset += sizeof(size_t);

    // Validate buffer size
    if (buffer.size() < offset + (command_count * sizeof(RecordedCmd)))
    {
        return false;
    }

    // Read commands
    session.commands.clear();
    session.commands.reserve(command_count);

    for (size_t i = 0; i < command_count; ++i)
    {
        RecordedCmd cmd;
        memcpy(&cmd, &buffer[offset], sizeof(RecordedCmd));
        session.commands.push_back(cmd);
        offset += sizeof(RecordedCmd);
    }

    return true;
}

// ============================================================================
// Recording Control Commands
// ============================================================================

void Cmd_Startrecord_f()
{
    if (is_recording)
    {
        CG_GameMessage(0, "^3Already recording");
        return;
    }

    const playerState_s *ps = CG_GetPredictedPlayerState(0);
    auto ca = &(*clients)[0];

    // Store initial position and angles
    current_session.startOrigin[0] = ps->origin[0];
    current_session.startOrigin[1] = ps->origin[1];
    current_session.startOrigin[2] = ps->origin[2];

    current_session.startAngles[0] = ca->viewangles[0];
    current_session.startAngles[1] = ca->viewangles[1];
    current_session.startAngles[2] = ca->viewangles[2];

    current_session.commands.clear();
    is_recording = true;
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

// ============================================================================
// Playback Control Commands
// ============================================================================

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

    if (current_session.commands.empty())
    {
        CG_GameMessage(0, "^1No recording available to play.\n");
        return;
    }

    // Teleport player to starting position
    TeleportPlayer(&g_entities[0], current_session.startOrigin, current_session.startAngles);

    play_frame = 0;
    is_playing = true;
    playback_start_time = 0; // Will be set on first UpdateCommand
    recording_start_time = current_session.commands[0].serverTime;
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

// ============================================================================
// File I/O Commands
// ============================================================================

void Cmd_Saverecord_f()
{
    // if (Cmd_Argc() < 2)
    // {
    //     CG_GameMessage(0, "^1Usage: saverecord <filename>");
    //     return;
    // }

    if (current_session.commands.empty())
    {
        CG_GameMessage(0, "^1No recording to save");
        return;
    }

    const char *filename = Cmd_Argv(1);

    // Create full path in game directory
    char full_path[256];
    sprintf(full_path, "game:\\recordings\\%s.rec", filename);

    // Serialize the recording session
    std::vector<char> buffer;
    if (!SerializeRecordingSession(current_session, buffer))
    {
        CG_GameMessage(0, "^1Failed to serialize recording");
        return;
    }

    // Write to disk
    if (filesystem::write_file_to_disk(full_path, buffer.data(), buffer.size()))
    {
        CG_GameMessage(0, "^2Recording saved successfully");
    }
    else
    {
        CG_GameMessage(0, "^1Failed to save recording");
    }
}

void Cmd_Loadrecord_f()
{
    // if (Cmd_Argc() < 2)
    // {
    //     CG_GameMessage(0, "^1Usage: loadrecord <filename>");
    //     return;
    // }

    if (is_recording)
    {
        CG_GameMessage(0, "^1Stop recording before loading");
        return;
    }

    if (is_playing)
    {
        CG_GameMessage(0, "^1Stop playback before loading");
        return;
    }

    const char *filename = Cmd_Argv(1);

    // Create full path in game directory
    char full_path[256];
    sprintf(full_path, "game:\\recordings\\%s.rec", filename);

    // Check if file exists
    if (!filesystem::file_exists(full_path))
    {
        CG_GameMessage(0, "^1Recording file not found");
        return;
    }

    // Read file contents
    std::string file_contents = filesystem::read_file_to_string(full_path);
    if (file_contents.empty())
    {
        CG_GameMessage(0, "^1Failed to read recording file");
        return;
    }

    // Convert to buffer
    std::vector<char> buffer(file_contents.begin(), file_contents.end());

    // Deserialize the recording session
    RecordingSession loaded_session;
    if (!DeserializeRecordingSession(buffer, loaded_session))
    {
        CG_GameMessage(0, "^1Failed to load recording - invalid format");
        return;
    }

    // Replace current session
    current_session = loaded_session;

    CG_GameMessage(0, "^2Recording loaded successfully");

    // Automatically start playback
    Cmd_Startplayback_f();
}

// ============================================================================
// Core Recording/Playback Logic
// ============================================================================

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

    short pitch = static_cast<short>(cmd->angles[PITCH]) + ANGLE2SHORT(ps->delta_angles[PITCH]);

    // Clamp pitch to 70 degrees down (positive pitch = looking down)
    // 70 degrees = 12743 units (32768 units = 180 degrees)
    // TODO: compute this based on the pitch dvar
    const short MAX_PITCH_DOWN = 12743;
    if (pitch > MAX_PITCH_DOWN)
        pitch = MAX_PITCH_DOWN;

    recorded_cmd.angles[PITCH] = pitch;
    recorded_cmd.angles[YAW] = static_cast<short>(cmd->angles[YAW]) + ANGLE2SHORT(ps->delta_angles[YAW]);
    recorded_cmd.angles[ROLL] = static_cast<short>(cmd->angles[ROLL]) + ANGLE2SHORT(ps->delta_angles[ROLL]);
    recorded_cmd.weapon = cmd->weapon;
    recorded_cmd.offHandIndex = cmd->offHandIndex;
    recorded_cmd.forwardmove = cmd->forwardmove;
    recorded_cmd.rightmove = cmd->rightmove;

    current_session.commands.push_back(recorded_cmd);
}

void UpdateCommand(usercmd_s *const cmd)
{
    if (current_session.commands.empty())
        return;

    if (play_frame >= current_session.commands.size())
    {
        Cmd_Stopplayback_f();
        return;
    }

    const playerState_s *ps = CG_GetPredictedPlayerState(0);
    auto ca = &(*clients)[0];
    const auto &data = current_session.commands[play_frame];

    // Initialize playback start time on first frame
    if (playback_start_time == 0)
    {
        playback_start_time = cmd->serverTime;
    }

    const int movement_threshold = 35;

    if (std::abs(cmd->forwardmove) >= movement_threshold || std::abs(cmd->rightmove) >= movement_threshold)
    {
        Cmd_Stopplayback_f();
        return;
    }

    // Calculate the relative time offset from the start of the recording
    int recording_time_offset = data.serverTime - recording_start_time;

    // Apply this offset to the current playback time
    cmd->serverTime = playback_start_time + recording_time_offset;

    // Merge buttons
    cmd->buttons |= data.buttons;

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

// ============================================================================
// Hook and Module Implementation
// ============================================================================

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

const float color_white_rgba[4] = {1.0f, 1.0f, 1.0f, 1.0f};

MovementRecorder::MovementRecorder()
{
    CL_CreateNewCommands_Detour = Detour(CL_CreateNewCommands, CL_CreateNewCommands_Hook);
    CL_CreateNewCommands_Detour.Install();

    // Register all commands
    Cmd_AddCommandInternal("startrecord", Cmd_Startrecord_f, &Cmd_Startrecord_VAR);
    Cmd_AddCommandInternal("stoprecord", Cmd_Stoprecord_f, &Cmd_Stoprecord_VAR);
    Cmd_AddCommandInternal("togglerecord", Cmd_Togglerecord_f, &Cmd_Togglerecord_VAR);
    Cmd_AddCommandInternal("startplayback", Cmd_Startplayback_f, &Cmd_Startplayback_VAR);
    Cmd_AddCommandInternal("stopplayback", Cmd_Stopplayback_f, &Cmd_Stopplayback_VAR);
    Cmd_AddCommandInternal("saverecord", Cmd_Saverecord_f, &Cmd_Saverecord_VAR);
    Cmd_AddCommandInternal("loadrecord", Cmd_Loadrecord_f, &Cmd_Loadrecord_VAR);

    // Register TAS indicator overlay
    Events::OnCG_DrawActive(
        []()
        {
            if (is_playing)
            {
                static auto bigDevFont = iw4::mp::R_RegisterFont("fonts/bigDevFont");
                iw4::mp::R_AddCmdDrawText("TAS", 4, bigDevFont, 10.f, 20.f, 1.0f, 1.0f, 0.0f, color_white_rgba, 0);
            }
        });
}

MovementRecorder::~MovementRecorder()
{
    CL_CreateNewCommands_Detour.Remove();
}
} // namespace mp
} // namespace iw4
