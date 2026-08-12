#pragma once

#include "pch.h"

namespace iw5
{
namespace mp
{
class PlayerMovement : public Module
{
  public:
    PlayerMovement();
    ~PlayerMovement();

    static void OnDvarInit();
};
} // namespace mp
} // namespace iw5
