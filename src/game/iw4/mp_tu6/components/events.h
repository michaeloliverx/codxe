#pragma once

#include "pch.h"

class Events : public Module
{
  public:
    Events();
    ~Events();

  private:
    static Detour Com_InitDvars_Detour;
    static void Com_InitDvars_Hook();

    static Detour CG_DrawActive_Detour;
    static void CG_DrawActive_Hook(int localClientNum);

    static Detour Cmd_Init_Detour;
    static void Cmd_Init_Hook();

    static Detour Scr_ShutdownSystem_Detour;
    static void Scr_ShutdownSystem_Hook(unsigned __int8 sys);
};
