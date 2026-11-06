#include "pch.h"
#include "mirror_world.h"

#include <cmath>

namespace iw3
{
namespace mp
{
namespace
{
// Row-vector convention: P' = P * H, VP' = VP * H, inverse(VP') = H * inverse(VP).
// H reflects clip-space X, preserving Y, Z, W and the camera's visible volume.
void ReflectClipX(float projection[4][4], float viewProjection[4][4], float inverseViewProjection[4][4])
{
    for (int i = 0; i < 4; ++i)
    {
        projection[i][0] = -projection[i][0];
        viewProjection[i][0] = -viewProjection[i][0];
        inverseViewProjection[0][i] = -inverseViewProjection[0][i];
    }
}

enum
{
    CULL_NONE = 0x4000,
    CULL_BACK = 0x8000,
    CULL_FRONT = 0xC000,
    CULL_MASK = 0xC000,
};

unsigned int CullStateBits(unsigned int bits, bool mirrored)
{
    const unsigned int cull = bits & CULL_MASK;
    if (mirrored && (cull == CULL_BACK || cull == CULL_FRONT))
        bits ^= CULL_NONE;
    return bits;
}

char MirrorMove(char move)
{
    const int mirrored = -static_cast<int>(static_cast<signed char>(move));
    return static_cast<char>(mirrored > 127 ? 127 : mirrored);
}

float AngleDelta(float before, float after)
{
    float delta = fmodf(after - before, 360.0f);
    if (delta > 180.0f)
        delta -= 360.0f;
    else if (delta < -180.0f)
        delta += 360.0f;
    return delta;
}

float MirrorAngle(float before, float after)
{
    return before - AngleDelta(before, after);
}

dvar_s *cg_mirrorWorld = nullptr;

bool IsEnabled()
{
    return cg_mirrorWorld && cg_mirrorWorld->current.enabled;
}

cg_s *currentViewWeapon = nullptr;

Detour AddViewWeaponDetour;
void CG_AddViewWeapon_Hook(int localClientNum)
{
    cg_s *previous = currentViewWeapon;
    currentViewWeapon = IsEnabled() ? &(*cgArray)[localClientNum] : nullptr;
    AddViewWeaponDetour.GetOriginal<decltype(CG_AddViewWeapon)>()(localClientNum);
    currentViewWeapon = previous;
}

Detour SaveWeaponOffsetDetour;
void CalculateWeaponPosition_SaveOffsetMovement_Hook(cg_s *cgameGlob, float *origin)
{
    if (currentViewWeapon == cgameGlob)
    {
        // Stock subtracts horizontal look sway before converting the position to world space.
        // Reverse that contribution while preserving movement/stance offsets and the weapon's normal pose.
        const float *side = cgameGlob->viewModelAxis[1];
        const float correction = 2.0f * cgameGlob->swayOffset[1];
        for (int i = 0; i < 3; ++i)
            origin[i] += correction * side[i];
    }

    // Save ADS offsets from the corrected position, while the sway/bob interpolation state stays untouched.
    SaveWeaponOffsetDetour.GetOriginal<decltype(CalculateWeaponPosition_SaveOffsetMovement)>()(cgameGlob, origin);
}

Detour WeaponAnglesDetour;
void BG_CalculateWeaponAngles_Hook(weaponState_t *ws, float *angles)
{
    WeaponAnglesDetour.GetOriginal<decltype(BG_CalculateWeaponAngles)>()(ws, angles);
    const cg_s *viewWeapon = currentViewWeapon;
    if (viewWeapon && ws->ps == &viewWeapon->predictedPlayerState)
    {
        // Stock subtracts swayAngles[YAW]. Reverse just that term; bob, recoil and roll keep their normal pose.
        // BG is also used by server aiming, so only adjust the active local first-person draw.
        angles[YAW] = AngleDelta(0.0f, angles[YAW] + 2.0f * ws->swayAngles[YAW]);
    }
}

// Command creation runs on the client thread; keep separate snapshots for split-screen clients.
struct CommandInput
{
    bool mirror;
    bool gamepadYaw;
    float yaw;
    float vehicleYaw;
};
CommandInput commandInputs[4] = {};

bool UpdateProjection(GfxCmdBufSourceState *source)
{
    if (source->viewMode != VIEW_MODE_3D || !source->viewParms3D)
        return false;

    // Main views use an infinite perspective projection. Spot shadows use finite perspective;
    // sun/cookie shadows use orthographic projection. cameraView alone misses auxiliary scene passes.
    const GfxMatrix &projection = source->viewParms3D->projectionMatrix;
    if (projection.m[2][3] != 1.0f || projection.m[2][2] != 0.0f)
        return false;

    const bool mirrored = IsEnabled() && source->depthHackFlags != GFX_DEPTH_RANGE_VIEWMODEL;
    const bool wasMirrored =
        source->viewParms.projectionMatrix.m[0][0] * source->viewParms3D->projectionMatrix.m[0][0] < 0.0f;
    if (mirrored != wasMirrored)
    {
        // Only the backend copy changes. DPVS, portal polygons and shadow cameras retain their original matrices.
        ReflectClipX(source->viewParms.projectionMatrix.m, source->viewParms.viewProjectionMatrix.m,
                     source->viewParms.inverseViewProjectionMatrix.m);

        // Invalidate projection, view-projection and world-view-projection, including their inverse/transpose forms.
        ++source->matrixVersions[2];
        ++source->matrixVersions[4];
        ++source->matrixVersions[5];
        R_DeriveNearPlaneConstantsForView(source);
    }
    return mirrored;
}

void UpdateCull(GfxCmdBufState *state, bool mirrored)
{
    // refStateBits stays in material space; activeStateBits records the actual GPU state.
    // Reapply on object transitions too: the weapon can share a material with mirrored geometry.
    const unsigned int bits = CullStateBits(state->refStateBits[0], mirrored);
    if (((bits ^ state->activeStateBits[0]) & CULL_MASK) != 0)
        R_ChangeState_0(state, bits);
}

void UpdateDrawContext(GfxCmdBufContext context)
{
    UpdateCull(context.state, UpdateProjection(context.source));
}

Detour SetupPassDetour;
void R_SetupPass_Hook(GfxCmdBufContext context, unsigned int passIndex)
{
    UpdateProjection(context.source);
    SetupPassDetour.GetOriginal<decltype(R_SetupPass)>()(context, passIndex);
    UpdateDrawContext(context);
}

// These run before the version checks and uploads. Depth-hack changes are inlined in several draw paths,
// so hooking R_ChangeDepthHackNearClip alone would miss weapon/world transitions.
Detour StableArgumentsDetour;
void R_SetPassVertexShaderStableArguments_Hook(GfxCmdBufContext context, unsigned int argCount,
                                               const MaterialShaderArgument *args)
{
    UpdateDrawContext(context);
    StableArgumentsDetour.GetOriginal<decltype(R_SetPassVertexShaderStableArguments)>()(context, argCount, args);
}

Detour ObjectArgumentsDetour;
void R_SetPassShaderObjectArguments_Hook(GfxCmdBufContext context, unsigned int argCount,
                                         const MaterialShaderArgument *args)
{
    UpdateDrawContext(context);
    ObjectArgumentsDetour.GetOriginal<decltype(R_SetPassShaderObjectArguments)>()(context, argCount, args);
}

Detour PrimArgumentsDetour;
void R_SetPassShaderPrimArguments_Hook(GfxCmdBufContext context, unsigned int argCount,
                                       const MaterialShaderArgument *args)
{
    UpdateDrawContext(context);
    PrimArgumentsDetour.GetOriginal<decltype(R_SetPassShaderPrimArguments)>()(context, argCount, args);
}

Detour CreateCmdDetour;
usercmd_s *CL_CreateCmd_Hook(usercmd_s *cmd, int localClientNum)
{
    CommandInput &input = commandInputs[localClientNum];
    const CommandInput previous = input;
    const clientActive_t *client = &(*clients)[localClientNum];
    input.mirror = IsEnabled() && clientUIActives[localClientNum].connectionState == CA_ACTIVE &&
                   !clientUIActives[localClientNum].keyCatchers;
    input.gamepadYaw = false;
    input.yaw = client->viewangles[YAW];
    input.vehicleYaw = client->vehicleViewYaw;
    usercmd_s *result = CreateCmdDetour.GetOriginal<decltype(CL_CreateCmd)>()(cmd, localClientNum);
    input = previous;
    return result;
}

Detour GamepadInputDetour;
void AimAssist_UpdateGamePadInput_Hook(const AimInput *input, AimOutput *output)
{
    CommandInput &command = commandInputs[input->localClientNum];
    if (command.mirror)
    {
        AimInput mirrored = *input;
        // Reflect keyboard/mouse yaw already accumulated this command, then the raw stick axes.
        // Aim assist still operates in world space and keeps its targeting corrections in the correct direction.
        mirrored.yaw = MirrorAngle(command.yaw, input->yaw);
        mirrored.yawAxis = -input->yawAxis;
        mirrored.rightAxis = -input->rightAxis;
        GamepadInputDetour.GetOriginal<decltype(AimAssist_UpdateGamePadInput)>()(&mirrored, output);
        command.gamepadYaw = true;
        return;
    }
    GamepadInputDetour.GetOriginal<decltype(AimAssist_UpdateGamePadInput)>()(input, output);
}

Detour FinishMoveDetour;
void CL_FinishMove_Hook(int localClientNum, usercmd_s *cmd)
{
    CommandInput &input = commandInputs[localClientNum];
    if (input.mirror)
    {
        clientActive_t *client = &(*clients)[localClientNum];
        // Reflect only this command's horizontal input, preserving the current heading and server delta angles.
        if (!input.gamepadYaw)
            client->viewangles[YAW] = MirrorAngle(input.yaw, client->viewangles[YAW]);
        client->vehicleViewYaw = MirrorAngle(input.vehicleYaw, client->vehicleViewYaw);
        cmd->rightmove = MirrorMove(cmd->rightmove);
        const int lean = cmd->buttons & (KEY_MASK_LEANLEFT | KEY_MASK_LEANRIGHT);
        cmd->buttons = (cmd->buttons & ~(KEY_MASK_LEANLEFT | KEY_MASK_LEANRIGHT)) | ((lean & KEY_MASK_LEANLEFT) << 1) |
                       ((lean & KEY_MASK_LEANRIGHT) >> 1);
        input.mirror = false;
    }
    FinishMoveDetour.GetOriginal<decltype(CL_FinishMove)>()(localClientNum, cmd);
}
} // namespace

void MirrorWorld::OnDvarInit()
{
    cg_mirrorWorld = Dvar_RegisterBool("cg_mirrorWorld", false, DVAR_FLAG_NONE,
                                       "Mirror the world horizontally while keeping the first-person weapon normal");
}

MirrorWorld::MirrorWorld()
{
    AddViewWeaponDetour = Detour(CG_AddViewWeapon, CG_AddViewWeapon_Hook);
    AddViewWeaponDetour.Install();
    SaveWeaponOffsetDetour =
        Detour(CalculateWeaponPosition_SaveOffsetMovement, CalculateWeaponPosition_SaveOffsetMovement_Hook);
    SaveWeaponOffsetDetour.Install();
    WeaponAnglesDetour = Detour(BG_CalculateWeaponAngles, BG_CalculateWeaponAngles_Hook);
    WeaponAnglesDetour.Install();
    SetupPassDetour = Detour(R_SetupPass, R_SetupPass_Hook);
    SetupPassDetour.Install();
    StableArgumentsDetour = Detour(R_SetPassVertexShaderStableArguments, R_SetPassVertexShaderStableArguments_Hook);
    StableArgumentsDetour.Install();
    ObjectArgumentsDetour = Detour(R_SetPassShaderObjectArguments, R_SetPassShaderObjectArguments_Hook);
    ObjectArgumentsDetour.Install();
    PrimArgumentsDetour = Detour(R_SetPassShaderPrimArguments, R_SetPassShaderPrimArguments_Hook);
    PrimArgumentsDetour.Install();
    CreateCmdDetour = Detour(CL_CreateCmd, CL_CreateCmd_Hook);
    CreateCmdDetour.Install();
    GamepadInputDetour = Detour(AimAssist_UpdateGamePadInput, AimAssist_UpdateGamePadInput_Hook);
    GamepadInputDetour.Install();
    FinishMoveDetour = Detour(CL_FinishMove, CL_FinishMove_Hook);
    FinishMoveDetour.Install();
}

MirrorWorld::~MirrorWorld()
{
    FinishMoveDetour.Remove();
    GamepadInputDetour.Remove();
    CreateCmdDetour.Remove();
    PrimArgumentsDetour.Remove();
    ObjectArgumentsDetour.Remove();
    StableArgumentsDetour.Remove();
    SetupPassDetour.Remove();
    WeaponAnglesDetour.Remove();
    SaveWeaponOffsetDetour.Remove();
    AddViewWeaponDetour.Remove();
}
} // namespace mp
} // namespace iw3
