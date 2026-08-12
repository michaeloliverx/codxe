#pragma once

#include "pch.h"

namespace iw4
{

namespace mp
{
class Events : public Module
{
  public:
    Events();
    ~Events();

  private:
    static Detour Com_InitDvars_Detour;
    static void Com_InitDvars_Hook();
};
} // namespace mp
} // namespace iw4
