#pragma once

#include "pch.h"

namespace iw3
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
} // namespace iw3
