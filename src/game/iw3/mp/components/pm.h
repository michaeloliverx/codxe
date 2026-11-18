#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class pm : public Module
{
  public:
    pm();

    static void OnDvarInit();
    static void OnCGDrawActive(int);
};
} // namespace mp
} // namespace iw3
