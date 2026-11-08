#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class Collision : public Module
{
  public:
    ~Collision();
    static void OnDvarInit();
    static void OnCGInit();
    static void OnCGDrawActive(int localClientNum);
};
} // namespace mp
} // namespace t4
