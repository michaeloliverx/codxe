#include "pch.h"
#include "cj_strafe.h"
#include "cj_strafe_target.h"
#include "command.h"

namespace iw3
{
namespace mp
{
namespace cj_strafe
{
namespace
{
struct ClientState
{
    bool held;
    int key;
    int side;
    int mode, lastTime, actionTime, action, targetSide;
    bool applied;
    const char *status;
    float appliedYaw, deltaYaw, offsetAnchor;
    Result result;
};
struct Target
{
    bool valid;
    float point[3];
};
ClientState states[4] = {};
Target targets[4] = {};
dvar_s *guide = nullptr;
dvar_s *mode = nullptr;
dvar_s *yawSpeed = nullptr;
struct Diagnostics
{
    int dpadDown, time;
    const char *status;
};
Diagnostics diagnostics[4] = {};

bool ValidClient(int localClientNum)
{
    return localClientNum >= 0 && localClientNum < static_cast<int>(ARRAYSIZE(states));
}

int CommandClient()
{
    const int nesting = cmd_args->nesting;
    return nesting >= 0 && nesting < 8 ? cmd_args->localClientNum[nesting] : -1;
}

void ResetMotion(ClientState &state)
{
    state.side = 0;
    state.applied = false;
    state.action = -1;
    state.targetSide = 0;
}

const char *DpadBinding(int client)
{
    const char *binding = playerKeys[client].keys[K_DPAD_RIGHT].binding;
    return binding ? binding : "";
}

void LogStatus(int client, const usercmd_s *cmd, int msec = 0)
{
    const ClientState &state = states[client];
    const playerState_s *ps = *cgArray && clientUIActives[client].connectionState == CA_ACTIVE
                                  ? &(*cgArray)[client].predictedPlayerState
                                  : nullptr;
    DbgPrint("[codxe][IW3][Strafe] client=%d mode=%d held=%d key=%d dpad=%d binding=\"%s\" "
             "status=\"%s\" cmd=%d psTime=%d msec=%d ground=%d pmType=%d pmFlags=%08X\n",
             client, mode ? mode->current.integer : 1, state.held, state.key,
             playerKeys[client].keys[K_DPAD_RIGHT].down, DpadBinding(client), state.status, cmd ? cmd->serverTime : -1,
             ps ? ps->commandTime : -1, msec, ps ? ps->groundEntityNum : -1, ps ? ps->pm_type : -1,
             ps ? ps->pm_flags : 0);
}

void Status(int client, const char *text, const usercmd_s *cmd, int msec = 0)
{
    ClientState &state = states[client];
    state.status = text;
    if (!state.held && (!mode || mode->current.integer != 2 || !targets[client].valid))
        return;
    Diagnostics &log = diagnostics[client];
    // State can alternate while client prediction catches up. Keep automatic logging bounded.
    if (cmd && log.status &&
        (!strcmp(log.status, text) || (cmd->serverTime >= log.time && cmd->serverTime - log.time < 250)))
        return;
    LogStatus(client, cmd, msec);
    log.status = text;
    log.time = cmd ? cmd->serverTime : 0;
}

void PrintStatus()
{
    const int client = CommandClient();
    if (!ValidClient(client))
        return;
    LogStatus(client, nullptr);
    char text[192];
    sprintf_s(text, "Strafe: %s; DPAD RIGHT = %.80s", states[client].status, DpadBinding(client));
    CG_GameMessage(client, text);
}

void SetTarget()
{
    const int client = CommandClient();
    if (!ValidClient(client) || clientUIActives[client].connectionState != CA_ACTIVE || !*cgArray)
        return;
    const refdef_s &view = (*cgArray)[client].refdef;
    const float zero[3] = {};
    float end[3];
    for (int axis = 0; axis < 3; ++axis)
        end[axis] = view.vieworg[axis] + view.viewaxis[0][axis] * 8192.0f;
    trace_t trace = {};
    CM_BoxTrace(&trace, view.vieworg, end, zero, zero, 0, 1);
    if (!Finite(trace.fraction) || trace.fraction <= 0.0f || trace.fraction >= 1.0f || trace.startsolid ||
        trace.allsolid || (trace.surfaceFlags & 4))
    {
        CG_GameMessage(client, "No world surface in view; target unchanged");
        return;
    }
    for (int axis = 0; axis < 3; ++axis)
        targets[client].point[axis] = view.vieworg[axis] + (end[axis] - view.vieworg[axis]) * trace.fraction;
    targets[client].valid = true;
    ResetMotion(states[client]);
    states[client].result.valid = false;
    // Saved controller layouts or a later profile exec can replace the CJ startup binding.
    // Marking a target explicitly restores the button allocated to this helper.
    DbgPrint("[codxe][IW3][Strafe] target client=%d xyz=(%.2f %.2f %.2f) DPAD_RIGHT=\"%s\"\n", client,
             targets[client].point[0], targets[client].point[1], targets[client].point[2], DpadBinding(client));
    if (strcmp(DpadBinding(client), "+cj_strafe"))
    {
        Cbuf_AddText(client, "bind DPAD_RIGHT \"+cj_strafe\"\n");
        DbgPrint("[codxe][IW3][Strafe] restoring DPAD_RIGHT binding for client=%d\n", client);
    }
    CG_GameMessage(client, "Air-strafe target set; hold DPAD RIGHT (cj_strafe_mode 2)");
}

void ClearTarget()
{
    const int client = CommandClient();
    if (ValidClient(client))
    {
        ResetClient(client, true);
        CG_GameMessage(client, "Air-strafe target cleared");
    }
}

void Press()
{
    const int localClientNum = CommandClient();
    if (!ValidClient(localClientNum) || states[localClientNum].held)
        return;
    if (mode && mode->current.integer == 2 && !targets[localClientNum].valid)
    {
        CG_GameMessage(localClientNum, "Aim at a surface and run cj_strafe_settarget first");
        return;
    }
    ResetClient(localClientNum);
    ClientState &state = states[localClientNum];
    state.held = true;
    const int nesting = cmd_args->nesting;
    if (cmd_args->argc[nesting] > 1)
        state.key = atoi(cmd_args->argv[nesting][1]);
    Status(localClientNum, "held; waiting for air command", nullptr);
}

void Release()
{
    const int localClientNum = CommandClient();
    if (ValidClient(localClientNum))
    {
        if (states[localClientNum].held)
            Status(localClientNum, "released", nullptr);
        ResetClient(localClientNum);
    }
}

// PM_Accelerate only reads this prefix of pml_t (frametime is at +36 in TU4).
struct AirFrame
{
    float forward[3], right[3], up[3];
    float frametime;
};
static_assert(offsetof(AirFrame, frametime) == 36, "");

struct Evaluation
{
    const playerState_s *source;
    playerState_s scratch;
    AirFrame frame;
    float offset, wishspeed;
    int forward, right;
    int msec, maxStep;
};

target::Motion EvaluateMotion(void *data, const target::Motion &start, float yaw, int forward, int right)
{
    Evaluation &evaluation = *static_cast<Evaluation *>(data);
    playerState_s &ps = evaluation.scratch;
    memcpy(ps.velocity, evaluation.source->velocity, sizeof(ps.velocity));
    memcpy(ps.oldVelocity, evaluation.source->oldVelocity, sizeof(ps.oldVelocity));
    ps.velocity[0] = start.x;
    ps.velocity[1] = start.y;
    ps.oldVelocity[0] = start.oldX;
    ps.oldVelocity[1] = start.oldY;
    const float offset = atan2f(-static_cast<float>(right), static_cast<float>(forward)) * DEGREES;
    const float angle = (yaw + offset) / DEGREES;
    const float wishdir[3] = {cosf(angle), sinf(angle), 0.0f};
    float wishspeed = static_cast<float>(ps.speed) * (abs(forward) > abs(right) ? abs(forward) : abs(right)) / 127.0f;
    if ((ps.pm_flags & 0x40) || ps.leanf != 0.0f)
        wishspeed *= 0.4f;
    const float view = yaw / DEGREES;
    evaluation.frame.forward[0] = cosf(view);
    evaluation.frame.forward[1] = sinf(view);
    evaluation.frame.right[0] = sinf(view);
    evaluation.frame.right[1] = -cosf(view);
    for (int remaining = evaluation.msec; remaining > 0;)
    {
        const int step = remaining < evaluation.maxStep ? remaining : evaluation.maxStep;
        evaluation.frame.frametime = step * 0.001f;
        if (forward || right)
            PM_Accelerate(&ps, reinterpret_cast<const pml_t *>(&evaluation.frame), wishdir, wishspeed, 1.0f);
        for (int axis = 0; axis < 2; ++axis)
            ps.oldVelocity[axis] += (ps.velocity[axis] - ps.oldVelocity[axis]) * evaluation.frame.frametime;
        Sys_SnapVector(ps.velocity);
        remaining -= step;
    }
    const target::Motion result = {ps.velocity[0], ps.velocity[1], ps.oldVelocity[0], ps.oldVelocity[1]};
    return result;
}

float Evaluate(void *data, float yaw)
{
    Evaluation &evaluation = *static_cast<Evaluation *>(data);
    const target::Motion start = {evaluation.source->velocity[0], evaluation.source->velocity[1],
                                  evaluation.source->oldVelocity[0], evaluation.source->oldVelocity[1]};
    const target::Motion result = EvaluateMotion(data, start, yaw, evaluation.forward, evaluation.right);
    return result.x * result.x + result.y * result.y;
}

void ApplyYaw(int client, usercmd_s &cmd, float yaw, float delta)
{
    cmd.angles[YAW] = NearestCommandYaw(yaw, delta);
    float &camera = (*clients)[client].viewangles[YAW];
    camera += Normalize(cmd.angles[YAW] * SHORT_ANGLE - camera);
}

bool Timing(const playerState_s &ps, const usercmd_s &cmd, int &msec, int &maxStep)
{
    maxStep = 66;
    __int64 finalTime = cmd.serverTime;
    const dvar_s *fixed = Dvar_FindMalleableVar("pm_fixed_fps_enable");
    if (fixed && fixed->current.enabled)
    {
        const dvar_s *fps = Dvar_FindMalleableVar("pm_fixed_fps");
        if (!fps || fps->current.integer <= 0 || fps->current.integer > 1000)
            return false;
        maxStep = 1000 / fps->current.integer;
        // Match the existing Pmove hook, including its minimum rounding increment.
        finalTime = ((finalTime + (maxStep < 2 ? 2 : maxStep) - 1) / maxStep) * maxStep;
    }
    const __int64 elapsed = finalTime - ps.commandTime;
    if (elapsed <= 0 || elapsed > 200)
        return false;
    msec = static_cast<int>(elapsed);
    return true;
}

void Rectangle(float x, float y, float width, float height, const float *color)
{
    R_AddCmdDrawStretchPic(x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, color, sharedUiInfo->assets.whiteMaterial);
}
} // namespace

void RegisterCommands()
{
    command::add("+cj_strafe", Press);
    command::add("-cj_strafe", Release);
    command::add("cj_strafe_settarget", SetTarget);
    command::add("cj_strafe_cleartarget", ClearTarget);
    command::add("cj_strafe_status", PrintStatus);
    Reset();
}

void RegisterDvars()
{
    guide = Dvar_RegisterBool("cj_strafe_guide", false, 0, "Show air-strafe yaw guidance");
    mode = Dvar_RegisterInt("cj_strafe_mode", 1, 0, 2, 0, "Air-strafe assist: 0 off, 1 stick, 2 target");
    yawSpeed =
        Dvar_RegisterInt("cj_strafe_yaw_speed", 180, 30, 360, 0, "Maximum assist yaw turn rate in degrees per second");
}

void ResetClient(int localClientNum, bool clearTarget)
{
    if (!ValidClient(localClientNum))
        return;
    memset(&states[localClientNum], 0, sizeof(states[localClientNum]));
    states[localClientNum].key = -1;
    states[localClientNum].action = -1;
    states[localClientNum].status = "hold DPAD RIGHT";
    if (clearTarget)
        targets[localClientNum].valid = false;
}

void Reset()
{
    for (int client = 0; client < static_cast<int>(ARRAYSIZE(states)); ++client)
    {
        ResetClient(client, true);
        diagnostics[client] = Diagnostics();
        diagnostics[client].dpadDown = -1;
    }
}

void UpdateCommand(int localClientNum, usercmd_s *cmd, bool playback, bool aimingRpg)
{
    if (!ValidClient(localClientNum))
        return;
    ClientState &state = states[localClientNum];
    state.result.valid = false;
    if (clientUIActives[localClientNum].keyCatchers || clientUIActives[localClientNum].connectionState != CA_ACTIVE)
    {
        if (state.held)
            Status(localClientNum, "released for menu/disconnect", cmd);
        ResetClient(localClientNum, clientUIActives[localClientNum].connectionState != CA_ACTIVE);
        return;
    }
    if (state.held && state.key >= 0 && state.key < 256 && !playerKeys[localClientNum].keys[state.key].down)
    {
        Status(localClientNum, "released: bound key is up", cmd);
        ResetClient(localClientNum);
    }
    const int currentMode = mode ? mode->current.integer : 1;
    if (currentMode == 2 && targets[localClientNum].valid)
    {
        const int down = playerKeys[localClientNum].keys[K_DPAD_RIGHT].down != 0;
        if (down != diagnostics[localClientNum].dpadDown)
        {
            diagnostics[localClientNum].dpadDown = down;
            DbgPrint("[codxe][IW3][Strafe] DPAD_RIGHT client=%d down=%d binding=\"%s\" held=%d\n", localClientNum, down,
                     DpadBinding(localClientNum), state.held);
        }
    }
    if (((!state.held || currentMode == 0) && (!guide || !guide->current.enabled)) || playback || aimingRpg || !cmd ||
        !*cgArray || !*clients)
    {
        Status(localClientNum,
               playback           ? "paused for TAS playback"
               : aimingRpg        ? "paused for RPG lookdown"
               : currentMode == 0 ? "assist off"
               : !state.held      ? "hold DPAD RIGHT"
                                  : "waiting for game state",
               cmd);
        if (playback || aimingRpg)
            ResetMotion(state);
        return;
    }
    if (state.mode != currentMode)
    {
        ResetMotion(state);
        state.mode = currentMode;
    }
    const bool targeting = currentMode == 2;
    if (targeting && !targets[localClientNum].valid)
    {
        Status(localClientNum, "set a target first", cmd);
        return;
    }

    const playerState_s &ps = (*cgArray)[localClientNum].predictedPlayerState;
    const int forward = static_cast<signed char>(cmd->forwardmove);
    const int right = static_cast<signed char>(cmd->rightmove);
    // Both modes use ordinary airborne commands. Target mode owns movement; stick mode preserves it.
    if (ps.pm_type != 0 || ps.groundEntityNum != 1023 || (ps.pm_flags & (4 | 8 | 0x20000 | 0x80000)) ||
        fabsf(Normalize(ps.viewangles[ROLL])) > 0.1f || ps.speed <= 0 ||
        (!targeting && abs(forward) < 20 && abs(right) < 20))
    {
        Status(localClientNum,
               ps.groundEntityNum != 1023                     ? "held; jump to steer"
               : ps.pm_type != 0                              ? "requires normal movement"
               : (ps.pm_flags & (4 | 8 | 0x20000 | 0x80000))  ? "movement restricted"
               : fabsf(Normalize(ps.viewangles[ROLL])) > 0.1f ? "requires zero roll"
               : ps.speed <= 0                                ? "invalid movement speed"
                                                              : "move the stick",
               cmd);
        ResetMotion(state);
        return;
    }

    Evaluation evaluation = {};
    if (!Timing(ps, *cmd, evaluation.msec, evaluation.maxStep) || evaluation.msec > evaluation.maxStep * 16)
    {
        Status(localClientNum, "waiting for command timing", cmd, evaluation.msec);
        return;
    }
    evaluation.source = &ps;
    evaluation.scratch = ps;
    evaluation.forward = forward;
    evaluation.right = right;
    evaluation.offset = atan2f(-static_cast<float>(right), static_cast<float>(forward)) * DEGREES;
    evaluation.wishspeed =
        static_cast<float>(ps.speed) * (abs(forward) > abs(right) ? abs(forward) : abs(right)) / 127.0f;
    if ((ps.pm_flags & 0x40) || ps.leanf != 0.0f)
        evaluation.wishspeed *= 0.4f;

    Input input = {};
    input.velocityX = ps.velocity[0];
    input.velocityY = ps.velocity[1];
    input.viewYaw = Normalize((cmd->angles[YAW] & 65535) * SHORT_ANGLE + ps.delta_angles[YAW]);
    input.deltaYaw = ps.delta_angles[YAW];
    input.forward = forward;
    input.right = right;
    input.wishspeed = evaluation.wishspeed;
    input.frametime = evaluation.msec * 0.001f;
    const dvar_s *stop = Dvar_FindMalleableVar("stopspeed");
    input.stopspeed = stop ? stop->current.value : 100.0f;

    if (state.applied && fabsf(Normalize(state.deltaYaw - input.deltaYaw)) > 0.01f)
        ResetMotion(state);
    float turnTime = Clamp(input.frametime, 0.0f, 0.05f);
    float fromYaw = input.viewYaw;
    if (state.held && state.applied)
    {
        const int elapsed = cmd->serverTime - state.lastTime;
        if (elapsed <= 0)
        {
            Status(localClientNum, "waiting for next command", cmd, evaluation.msec);
            return;
        }
        if (elapsed > 100)
            ResetMotion(state);
        else
        {
            turnTime = Clamp(elapsed * 0.001f, 0.0f, 0.05f);
            fromYaw = state.appliedYaw;
        }
    }
    const float turnRate = yawSpeed ? static_cast<float>(yawSpeed->current.integer) : 180.0f;

    if (targeting)
    {
        target::Input plan = {};
        plan.velocity.x = ps.velocity[0];
        plan.velocity.y = ps.velocity[1];
        plan.velocity.oldX = ps.oldVelocity[0];
        plan.velocity.oldY = ps.oldVelocity[1];
        plan.x = ps.origin[0];
        plan.y = ps.origin[1];
        plan.z = ps.origin[2];
        plan.verticalSpeed = ps.velocity[2];
        plan.gravity = static_cast<float>(ps.gravity);
        plan.targetX = targets[localClientNum].point[0];
        plan.targetY = targets[localClientNum].point[1];
        plan.targetZ = targets[localClientNum].point[2];
        plan.yaw = fromYaw;
        plan.deltaYaw = input.deltaYaw;
        plan.yawSpeed = turnRate;
        plan.frametime = input.frametime;
        plan.speed = ps.speed * (((ps.pm_flags & 0x40) || ps.leanf != 0.0f) ? 0.4f : 1.0f);
        plan.stopspeed = input.stopspeed;
        plan.previousAction = state.action;
        plan.previousSide = state.targetSide;
        plan.lockAction = state.held && state.action >= 0 && cmd->serverTime - state.actionTime < 120;
        const target::Result selected = target::Solve(plan, EvaluateMotion, &evaluation);
        if (!selected.valid)
        {
            Status(localClientNum, "no valid target plan", cmd, evaluation.msec);
            return;
        }
        state.result.valid = true;
        state.result.canGain = true;
        state.result.yaw = selected.goalYaw;
        state.result.bandLow = Normalize(selected.goalYaw - 1.0f);
        state.result.bandHigh = Normalize(selected.goalYaw + 1.0f);
        if (state.held)
        {
            if (state.action != selected.action || state.targetSide != selected.side)
                state.actionTime = cmd->serverTime;
            state.action = selected.action;
            state.targetSide = selected.side;
            const float applied = LimitedYaw(fromYaw, selected.yaw, turnRate * turnTime, input.deltaYaw);
            ApplyYaw(localClientNum, *cmd, applied, input.deltaYaw);
            cmd->forwardmove = static_cast<char>(selected.forward);
            cmd->rightmove = static_cast<char>(selected.right);
        }
    }
    else
    {
        // Changing movement quadrant selects a new nearby branch instead of compensating with a camera flip.
        if (!state.side || fabsf(Normalize(evaluation.offset - state.offsetAnchor)) > 45.0f)
        {
            state.side = 0;
            state.offsetAnchor = evaluation.offset;
        }
        input.preferredSide = state.side;
        input.lockSide = state.held;
        state.result = Solve(input, Evaluate, &evaluation);
        if (!state.result.valid)
        {
            Status(localClientNum, "no valid strafe angle", cmd, evaluation.msec);
            return;
        }
        state.side = state.result.side;
        if (state.held && currentMode != 0 && state.result.canGain)
        {
            float desired = state.result.yaw;
            // Stay in a good snapped-speed band instead of chasing tiny changes in the peak.
            if (state.result.currentGain >= state.result.gain * 0.9f)
                desired = input.viewYaw;
            else
            {
                const float low = Normalize(state.result.bandLow - input.viewYaw);
                const float high = low + Normalize(state.result.bandHigh - state.result.bandLow);
                desired = fabsf(low) < fabsf(high) ? state.result.bandLow : state.result.bandHigh;
            }
            ApplyYaw(localClientNum, *cmd, LimitedYaw(fromYaw, desired, turnRate * turnTime, input.deltaYaw),
                     input.deltaYaw);
        }
    }
    if (state.held && currentMode != 0)
    {
        Status(localClientNum, targeting ? "steering" : "assisting", cmd, evaluation.msec);
        state.applied = true;
        state.lastTime = cmd->serverTime;
        state.deltaYaw = input.deltaYaw;
        state.appliedYaw = Normalize((cmd->angles[YAW] & 65535) * SHORT_ANGLE + input.deltaYaw);
    }
}

void Draw(int localClientNum)
{
    if (!ValidClient(localClientNum) || clientUIActives[localClientNum].connectionState != CA_ACTIVE ||
        clientUIActives[localClientNum].keyCatchers || !*cgArray || !*clients)
        return;
    const ClientState &state = states[localClientNum];
    const bool targeting = mode && mode->current.integer == 2 && targets[localClientNum].valid;
    const bool showGuide = guide && guide->current.enabled && state.result.valid;
    if (!targeting && !showGuide)
        return;
    const cg_s &cg = (*cgArray)[localClientNum];
    Font_s *font = sharedUiInfo->assets.smallFont;
    if (!font || font->pixelHeight <= 0 || !sharedUiInfo->assets.whiteMaterial || !cg.refdef.width || !cg.refdef.height)
        return;

    const float scale =
        (cg.refdef.width / 640.0f < cg.refdef.height / 480.0f) ? cg.refdef.width / 640.0f : cg.refdef.height / 480.0f;
    const float center = cg.refdef.x + cg.refdef.width * 0.5f;
    const float y = cg.refdef.y + cg.refdef.height * 0.65f;
    const float halfWidth = 130.0f * scale;
    const float yaw = Normalize((*clients)[localClientNum].viewangles[YAW] + cg.predictedPlayerState.delta_angles[YAW]);
    const float error = Normalize(state.result.yaw - yaw);
    const float low = Normalize(state.result.bandLow - yaw);
    const float high = low + Normalize(state.result.bandHigh - state.result.bandLow);
    const bool aligned = low <= 0.0f && high >= 0.0f;
    const float white[4] = {1.0f, 1.0f, 1.0f, 0.95f};
    const float track[4] = {0.1f, 0.1f, 0.1f, 0.7f};
    const float green[4] = {0.3f, 1.0f, 0.4f, 0.95f};
    const float amber[4] = {1.0f, 0.75f, 0.25f, 0.95f};
    if (targeting)
    {
        const float *point = targets[localClientNum].point;
        float projected[3] = {};
        for (int axis = 0; axis < 3; ++axis)
            for (int component = 0; component < 3; ++component)
                projected[axis] +=
                    (point[component] - cg.refdef.vieworg[component]) * cg.refdef.viewaxis[axis][component];
        if (projected[0] > 1.0f && cg.refdef.tanHalfFovX > 0.0f && cg.refdef.tanHalfFovY > 0.0f)
        {
            const float x = center - projected[1] * cg.refdef.width / (2.0f * projected[0] * cg.refdef.tanHalfFovX);
            const float markerY = cg.refdef.y + cg.refdef.height * 0.5f -
                                  projected[2] * cg.refdef.height / (2.0f * projected[0] * cg.refdef.tanHalfFovY);
            if (x >= cg.refdef.x + 8.0f * scale && x <= cg.refdef.x + cg.refdef.width - 8.0f * scale &&
                markerY >= cg.refdef.y + 8.0f * scale && markerY <= cg.refdef.y + cg.refdef.height - 8.0f * scale)
            {
                Rectangle(x - 6.0f * scale, markerY - scale, 12.0f * scale, 2.0f * scale, green);
                Rectangle(x - scale, markerY - 6.0f * scale, 2.0f * scale, 12.0f * scale, green);
            }
        }
    }
    if (showGuide)
        Rectangle(center - halfWidth, y, halfWidth * 2.0f, 6.0f * scale, track);
    if (showGuide && state.result.canGain)
    {
        const float left = center - Clamp(high, -60.0f, 60.0f) * halfWidth / 60.0f;
        const float right = center - Clamp(low, -60.0f, 60.0f) * halfWidth / 60.0f;
        Rectangle(left, y, right - left > 2.0f * scale ? right - left : 2.0f * scale, 6.0f * scale, green);
    }
    if (showGuide)
        Rectangle(center - scale, y - 3.0f * scale, 2.0f * scale, 12.0f * scale, white);
    char text[96];
    if (targeting)
    {
        const float dx = targets[localClientNum].point[0] - cg.predictedPlayerState.origin[0];
        const float dy = targets[localClientNum].point[1] - cg.predictedPlayerState.origin[1];
        const float distance = sqrtf(dx * dx + dy * dy);
        const bool landedNearby = distance <= 32.0f && cg.predictedPlayerState.groundEntityNum != 1023 &&
                                  fabsf(cg.predictedPlayerState.origin[2] - targets[localClientNum].point[2]) <= 24.0f;
        sprintf_s(text, "TARGET  %.0f units  %s", distance,
                  landedNearby ? "landed nearby"
                  : state.held ? state.status
                               : "hold DPAD RIGHT");
    }
    else if (!state.result.canGain)
        sprintf_s(text, "AIR STRAFE  no speed gain with this input");
    else if (state.held && (!mode || mode->current.integer != 0))
        sprintf_s(text, "AIR STRAFE  assisting  yaw %.1f", state.result.yaw);
    else if (aligned)
        sprintf_s(text, "AIR STRAFE  on target  yaw %.1f", state.result.yaw);
    else
        sprintf_s(text, "AIR STRAFE  %s %.1f  yaw %.1f", error > 0.0f ? "LEFT" : "RIGHT", fabsf(error),
                  state.result.yaw);
    const float fontScale = 14.0f * scale / font->pixelHeight;
    R_AddCmdDrawText(text, sizeof(text), font, center - halfWidth, y + 25.0f * scale, fontScale, fontScale, 0.0f,
                     targeting || (aligned && state.result.canGain) ? green : amber, 0);
}
} // namespace cj_strafe
} // namespace mp
} // namespace iw3
