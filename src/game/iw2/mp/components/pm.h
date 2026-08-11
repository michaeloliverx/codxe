#pragma once
#include "pch.h"

namespace iw2
{
namespace mp
{

typedef float vec_t;
typedef vec_t vec3_t[3];

namespace PV_Config
{
static const uint32_t PatchAddr = 0x8248B178; // Location where we should call our dummy function
static const uint32_t DummyAddr =
    0x824550B8; // In COD2 there is no call to clip_velocity so we will use a dummy function address (and hook to it)
static const uint32_t CallToDummyAddr = 0x82455F74; // Location of calls to unused function
} // namespace PV_Config
// Forward declaration using float* to avoid array-size decay errors
void PM_ProjectVelocity_Hook(vec3_t in, vec3_t normal, vec3_t out);

class PlayerMovement : public Module
{
  public:
    PlayerMovement();
    virtual ~PlayerMovement();

  private:
    void install_patch();
    PlayerMovement(const PlayerMovement &);
    PlayerMovement &operator=(const PlayerMovement &);
};

} // namespace mp
} // namespace iw2
