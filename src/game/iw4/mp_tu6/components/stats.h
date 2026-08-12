#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class stats : public Module
{
  public:
    stats();
    ~stats();

    static void OnCmdInit();
};
} // namespace mp_tu6
} // namespace iw4
