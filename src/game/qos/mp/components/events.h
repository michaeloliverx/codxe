#pragma once

#include "pch.h"

namespace qos
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

    static void OnCG_DrawActive(const std::function<void()> &callback);
    static void OnCG_Init(const std::function<void()> &callback);
    static void OnVMShutdown(const std::function<void()> &callback);

  private:
    static std::vector<std::function<void()>> cg_drawactive_callbacks;
    static Detour CG_DrawActive_Detour;
    static void CG_DrawActive_Hook(int localClientNum);

    static std::vector<std::function<void()>> cg_init_callbacks;
    static Detour CG_Init_Detour;
    static void CG_Init_Hook(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum);

    static std::vector<std::function<void()>> vmshutdown_callbacks;
    static Detour Scr_ShutdownSystem_Detour;
    static void Scr_ShutdownSystem_Hook(unsigned __int8 sys, int bComplete);
};
} // namespace mp
} // namespace qos