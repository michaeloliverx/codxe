#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class MirrorWorld : public Module
{
  public:
    MirrorWorld();

    static void OnDvarInit();
};
} // namespace mp
} // namespace iw3
