#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{

class Events : public Module
{
  public:
    Events();
    ~Events();

    const char *get_name() override
    {
        return "Events";
    };

  private:
    static Detour CG_DrawActive_Detour;
    static void CG_DrawActive_Hook(int localClientNum);

    static Detour CG_Init_Detour;
    static void CG_Init_Hook(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum);

    static Detour Scr_ShutdownSystem_Detour;
    static void Scr_ShutdownSystem_Hook(unsigned __int8 sys);

    static Detour Com_InitDvars_Detour;
    static void Com_InitDvars_Hook();

    static Detour Cmd_Init_Detour;
    static void Cmd_Init_Hook();

    static Detour UI_Refresh_Detour;
    static void UI_Refresh_Hook(int localClientNum);
};
} // namespace mp
} // namespace iw3
