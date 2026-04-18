#include "pch.h"
#include "project_velocity.h"
#include <cmath>

namespace ngl {
namespace mp {

// Global detour object
Detour PM_ProjectVelocity_Detour;

const uint32_t NOP_INST = 0x60000000;

// The Hook function that the Detour will point to
void PM_ProjectVelocity_Hook(vec3_t in, vec3_t normal, vec3_t out)
{
    const float EPSILON = 0.001f;
    float normalZ = normal[2];
    float speedXY = (in[0] * in[0]) + (in[1] * in[1]);

    if (normalZ < EPSILON || speedXY <= 0.0f) {
        out[0] = in[0]; out[1] = in[1]; out[2] = in[2];
        return;
    }

    float dotNormalXY = (in[0] * normal[0]) + (in[1] * normal[1]);
    float projRatio   = -dotNormalXY / normalZ;
    float projSq      = (in[2] * in[2]) + speedXY;
    float divisor     = speedXY + (projRatio * projRatio);

    if (divisor > 0.0f) {
        float scale = sqrtf(projSq / divisor);
        // FIXED: Added logical OR operators
        if (scale < 1.0f || projRatio < 0.0f || in[2] > 0.0f) {
            out[0] = in[0] * scale;
            out[1] = in[1] * scale;
            out[2] = projRatio * scale;
            return;
        }
    }

    out[0] = in[0]; out[1] = in[1]; out[2] = in[2];
}

// Simple helper to build the branch-link (bl) instruction
static uint32_t make_bl(uint32_t current, uint32_t target) {
    uint32_t li = (target - current) & 0x03FFFFFC;
    return (18u << 26) | li | 1u;
}

// Simple helper to build the unconditional branch (b) instruction
//static uint32_t make_b(uint32_t current, uint32_t target) {
//    uint32_t li = (target - current) & 0x03FFFFFC;
//    return (18u << 26) | li;
//}

void project_velocity::install_patch() {
    // 1. NOP out the original call to the dummy function
    *(volatile uint32_t*)PV_Config::CallToDummyAddr = NOP_INST;

    // 2. Initialize the Detour on the DummyAddr
    // This redirects any call to DummyAddr to our PM_ProjectVelocity_Hook
    PM_ProjectVelocity_Detour = Detour(reinterpret_cast<void*>(PV_Config::DummyAddr), reinterpret_cast<void*>(PM_ProjectVelocity_Hook));
    PM_ProjectVelocity_Detour.Install();

    // 3. Write the inline argument setup and call at PatchAddr
    volatile uint32_t* patch = (volatile uint32_t*)PV_Config::PatchAddr;

    patch[0] = make_bl(PV_Config::PatchAddr, PV_Config::DummyAddr);
}

project_velocity::project_velocity() {
    install_patch();
}

project_velocity::~project_velocity() {
    PM_ProjectVelocity_Detour.Remove();
}

} // namespace mp
} // namespace ngl