#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class clipmap : public Module
{
  public:
    clipmap();
    ~clipmap();

    static void OnDvarInit();
    static void OnCGInit();
    static void OnCGDrawActive();

  private:
    static void HandleBrushCollisionChange();
};
} // namespace mp
} // namespace iw3
