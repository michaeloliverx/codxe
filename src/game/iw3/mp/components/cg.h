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
    static void OnCGDrawActive();
};
} // namespace mp
} // namespace iw3
