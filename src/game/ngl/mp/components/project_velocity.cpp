#include "pch.h"
#include "project_velocity.h"
#include <cmath>

namespace ngl {
namespace mp {

// Logic for the math
void PM_ProjectVelocity(vec3_t in, vec3_t normal, vec3_t out)
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
        if (scale < 1.0f || projRatio < 0.0f || in[2] > 0.0f) {
            out[0] = in[0] * scale;
            out[1] = in[1] * scale;
            out[2] = projRatio * scale;
            return;
        }
    }
    out[0] = in[0]; out[1] = in[1]; out[2] = in[2];
}

static uint32_t make_bl(uint32_t current, uint32_t target) {
    uint32_t li = (target - current) & 0x03FFFFFC;
    return (18u << 26) | li | 1u;
}

void project_velocity::install_patch() {
    uint32_t funcAddr = (uint32_t)(void*)&PM_ProjectVelocity;

    uint32_t NOP_INST = 0x60000000;

    // Explicitly cast the value to (uint32_t) to ensure the compiler
    // treats it as a raw bit pattern before assigning it to the volatile pointer.
    *(volatile uint32_t *)PV_Config::CallToStubAddr = (uint32_t)NOP_INST;

    volatile uint32_t* stub = (volatile uint32_t*)PV_Config::StubAddr;
    uint16_t hi = (uint16_t)((funcAddr >> 16) & 0xFFFF);
    uint16_t lo = (uint16_t)(funcAddr & 0xFFFF);

    stub[0] = 0x3D800000 | hi;
    stub[1] = 0x618C0000 | lo;
    stub[2] = 0x7D8903A6;
    stub[3] = 0x4E800420;

    *(volatile uint32_t*)PV_Config::PatchAddr = make_bl(PV_Config::PatchAddr, PV_Config::StubAddr);
}

project_velocity::project_velocity() {
    install_patch();
}

project_velocity::~project_velocity() {}

} // namespace mp
} // namespace ngl