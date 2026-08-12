#pragma once

#include "pch.h"

namespace iw5
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
    static Detour G_ShutdownGame_Detour;
    static void Com_InitDvars_Hook();
    static void G_ShutdownGame_Hook(int freeScripts);
};
} // namespace mp
} // namespace iw5
