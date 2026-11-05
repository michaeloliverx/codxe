#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class Demo : public Module
{
  public:
    Demo();
    ~Demo();

    static void OnDvarInit();
};
} // namespace mp
} // namespace iw3
