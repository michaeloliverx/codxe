#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class stats : public Module
{
  public:
    stats();
    ~stats();

    static void OnCmdInit();
};
} // namespace mp
} // namespace t4
