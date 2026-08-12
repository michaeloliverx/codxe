#pragma once

#include "pch.h"

namespace iw4
{

namespace mp
{
class pm : public Module
{
  public:
    pm();
    ~pm();

    static void OnDvarInit();
};
} // namespace mp
} // namespace iw4
