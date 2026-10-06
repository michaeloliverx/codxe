#pragma once

#include "pch.h"

namespace t4
{
namespace sp
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
} // namespace sp
} // namespace t4
