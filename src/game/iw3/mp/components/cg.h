#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class cg : public Module
{
  public:
    cg();
    ~cg();

    static void OnDvarInit();
    static void OnCGDrawActive(int localClientNum);
};
} // namespace mp
} // namespace iw3
