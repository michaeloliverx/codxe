#pragma once

#include "pch.h"

namespace t4
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

    static Detour Cmd_Init_Detour;
    static void Cmd_Init_Hook();

    static Detour Scr_ShutdownSystem_Detour;
    static void *Scr_ShutdownSystem_Hook(scriptInstance_t inst, int sys, int bComplete);

    static Detour UI_Refresh_Detour;
    static int UI_Refresh_Hook(int localClientNum);
};
} // namespace mp
} // namespace t4
