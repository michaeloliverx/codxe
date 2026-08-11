#include "pch.h"
#include "pm.h"
#include <cmath>

namespace ngl
{
namespace mp
{

namespace
{
cvar_t *bg_bounces = nullptr;

Detour Cvar_RegisterCommands_Detour;
Detour PM_ProjectVelocity_Detour;

void Cvar_RegisterCommands_Hook()
{
    Cvar_RegisterCommands_Detour.GetOriginal<Cvar_RegisterCommands_t>()();

    // Bounce glitch settings
    bg_bounces = Cvar_Get("bg_bounces", "0", CVAR_SYSTEMINFO);
}

void PM_ProjectVelocity(const vec3_t in, const vec3_t normal, vec3_t out)
{
    if (bg_bounces == nullptr || bg_bounces->integer == 0)
    {
        PM_ClipVelocity(in, normal, out);
        return;
    }

    const float EPSILON = 0.001f;
    const float normalZ = normal[2];
    const float absoluteNormalZ = fabsf(normalZ);
    const float speedXY = (in[0] * in[0]) + (in[1] * in[1]);

    if (absoluteNormalZ < EPSILON || speedXY <= 0.0f)
    {
        out[0] = in[0];
        out[1] = in[1];
        out[2] = in[2];
        return;
    }

    const float dotNormalXY = (in[0] * normal[0]) + (in[1] * normal[1]);
    const float projRatio = -dotNormalXY / normalZ;
    const float projSq = (in[2] * in[2]) + speedXY;
    const float scale = sqrtf(projSq / (speedXY + (projRatio * projRatio)));

    if (scale < 1.0f || projRatio < 0.0f || in[2] > 0.0f)
    {
        out[0] = in[0] * scale;
        out[1] = in[1] * scale;
        out[2] = projRatio * scale;
        return;
    }

    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}

__declspec(naked) void PM_ProjectVelocity_Hook()
{
    __asm
    {
        stw r12, -0x10(r1)
        mflr r12
        stw r12, -8(r1)
        stwu r1, -0x90(r1)
        stw r10, 0x50(r1)
        stfd fr6, 0x58(r1)
        stfd fr7, 0x60(r1)
        stfd fr8, 0x68(r1)
        stfd fr9, 0x70(r1)

        bl PM_ProjectVelocity

        lfd fr6, 0x58(r1)
        lfd fr7, 0x60(r1)
        lfd fr8, 0x68(r1)
        lfd fr9, 0x70(r1)
        lwz r10, 0x50(r1)
        addi r1, r1, 0x90
        lwz r0, -8(r1)
        mtlr r0
        lwz r12, -0x10(r1)
        blr
    }
}
} // namespace

static uint32_t make_bl(uint32_t current, uint32_t target)
{
    uint32_t li = (target - current) & 0x03FFFFFC;
    return (18u << 26) | li | 1u;
}

void PlayerMovement::install_patch()
{
    const uint32_t patchAddr = 0x8246E1E0;
    const uint32_t hookTargetAddr = 0x8244C1C8;
    const uint32_t callToHookTargetAddr = 0x82466704;
    const uint32_t nop = 0x60000000;

    *(volatile uint32_t *)callToHookTargetAddr = nop;

    PM_ProjectVelocity_Detour =
        Detour(reinterpret_cast<void *>(hookTargetAddr), reinterpret_cast<void *>(PM_ProjectVelocity_Hook));
    PM_ProjectVelocity_Detour.Install();

    volatile uint32_t *patch = reinterpret_cast<volatile uint32_t *>(patchAddr);
    patch[0] = make_bl(patchAddr, hookTargetAddr);
}

PlayerMovement::PlayerMovement()
{
    Cvar_RegisterCommands_Detour = Detour(Cvar_RegisterCommands, Cvar_RegisterCommands_Hook);
    Cvar_RegisterCommands_Detour.Install();

    install_patch();
}

PlayerMovement::~PlayerMovement()
{
    PM_ProjectVelocity_Detour.Remove();
    Cvar_RegisterCommands_Detour.Remove();
    bg_bounces = nullptr;
}

} // namespace mp
} // namespace ngl
