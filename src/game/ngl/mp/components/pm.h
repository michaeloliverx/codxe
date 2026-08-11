#pragma once
#include "pch.h"

namespace ngl
{
namespace mp
{

typedef float vec_t;
typedef vec_t vec3_t[3];

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
} // namespace ngl
