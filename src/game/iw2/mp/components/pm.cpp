#include "pch.h"
#include "pm.h"
#include <cmath>

namespace iw2
{
namespace mp
{

namespace
{
dvar_s *bg_bounces = nullptr;

Detour Jump_RegisterDvars_Detour;
Detour PM_ProjectVelocity_Detour;

const uint32_t NOP_INST = 0x60000000;

void Jump_RegisterDvars_Hook()
{
    Jump_RegisterDvars_Detour.GetOriginal<Jump_RegisterDvars_t>()();

    bg_bounces = Dvar_RegisterVariant("bg_bounces", DVAR_TYPE_BOOL, DVAR_FLAG_SERVERINFO, false, 0);
}

void PM_ClipVelocity(const vec3_t in, const vec3_t normal, vec3_t out)
{
    const float dot = in[1] * normal[1] + (in[2] * normal[2] + in[0] * normal[0]);
    const float backoff = fabsf(dot) * 0.001f - dot;

    out[0] = normal[0] * backoff + in[0];
    out[1] = normal[1] * backoff + in[1];
    out[2] = normal[2] * backoff + in[2];
}
} // namespace

void PM_ProjectVelocity_Hook(vec3_t in, vec3_t normal, vec3_t out)
{
    if (bg_bounces == nullptr || !bg_bounces->current.enabled)
    {
        PM_ClipVelocity(in, normal, out);
        return;
    }

    const float EPSILON = 0.001f;
    float normalZ = normal[2];
    float absoluteNormalZ = normalZ < 0.0f ? -normalZ : normalZ;
    float speedXY = (in[0] * in[0]) + (in[1] * in[1]);

    if (absoluteNormalZ < EPSILON || speedXY <= 0.0f)
    {
        out[0] = in[0];
        out[1] = in[1];
        out[2] = in[2];
        return;
    }

    float dotNormalXY = (in[0] * normal[0]) + (in[1] * normal[1]);
    float projRatio = -dotNormalXY / normalZ;
    float projSq = (in[2] * in[2]) + speedXY;
    float divisor = speedXY + (projRatio * projRatio);

    if (divisor > 0.0f)
    {
        float scale = sqrtf(projSq / divisor);
        // FIXED: Added logical OR operators
        if (scale < 1.0f || projRatio < 0.0f || in[2] > 0.0f)
        {
            out[0] = in[0] * scale;
            out[1] = in[1] * scale;
            out[2] = projRatio * scale;
            return;
        }
    }

    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}

// Simple helper to build the branch-link (bl) instruction
static uint32_t make_bl(uint32_t current, uint32_t target)
{
    uint32_t li = (target - current) & 0x03FFFFFC;
    return (18u << 26) | li | 1u;
}

// Simple helper to build the unconditional branch (b) instruction
static uint32_t make_b(uint32_t current, uint32_t target)
{
    uint32_t li = (target - current) & 0x03FFFFFC;
    return (18u << 26) | li;
}

void PlayerMovement::install_patch()
{
    // 1. NOP out the original call to the dummy function
    *(volatile uint32_t *)PV_Config::CallToDummyAddr = NOP_INST;

    // 2. Initialize the Detour on the DummyAddr
    // This redirects any call to DummyAddr to our PM_ProjectVelocity_Hook
    PM_ProjectVelocity_Detour =
        Detour(reinterpret_cast<void *>(PV_Config::DummyAddr), reinterpret_cast<void *>(PM_ProjectVelocity_Hook));
    PM_ProjectVelocity_Detour.Install();

    // 3. Write the inline argument setup and call at PatchAddr
    volatile uint32_t *patch = (volatile uint32_t *)PV_Config::PatchAddr;

    // Setup Arguments
    patch[0] = 0x7FE3FB78; // mr r3, r31
    patch[1] = 0x388100A4; // addi r4, r1, 0xA4
    patch[2] = 0x7FE5FB78; // mr r5, r31

    // Call the hooked DummyAddr (where our Detour is waiting)
    uint32_t blAddr = PV_Config::PatchAddr + (3 * 4);
    patch[3] = make_bl(blAddr, PV_Config::DummyAddr);

    // Jump past the rest of the original logic (18 instructions total)
    uint32_t bAddr = PV_Config::PatchAddr + (4 * 4);
    uint32_t targetAddr = PV_Config::PatchAddr + (18 * 4);
    patch[4] = make_b(bAddr, targetAddr);

    // Clear the remaining original instructions
    for (int i = 5; i < 18; i++)
    {
        patch[i] = NOP_INST;
    }
}

PlayerMovement::PlayerMovement()
{
    Jump_RegisterDvars_Detour = Detour(Jump_RegisterDvars, Jump_RegisterDvars_Hook);
    Jump_RegisterDvars_Detour.Install();

    install_patch();
}

PlayerMovement::~PlayerMovement()
{
    PM_ProjectVelocity_Detour.Remove();
    Jump_RegisterDvars_Detour.Remove();
    bg_bounces = nullptr;
}

} // namespace mp
} // namespace iw2
