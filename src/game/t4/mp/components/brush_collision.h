#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class BrushCollision : public Module
{
  public:
    BrushCollision();
    ~BrushCollision();

    static void OnDvarInit();
};
} // namespace mp
} // namespace t4
