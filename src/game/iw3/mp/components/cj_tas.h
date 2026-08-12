#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class cj_tas : public Module
{
  public:
    cj_tas();
    ~cj_tas();

    static void OnDvarInit();
    static void OnCGDrawActive();
    static void OnCGInit();
    static bool TAS_Enabled();
};
} // namespace mp
} // namespace iw3
