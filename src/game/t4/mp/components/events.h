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

    const char *get_name() override
    {
        return "Events";
    };

    static void OnDvarInit(const std::function<void()> &callback);
    static void OnCmdInit(const std::function<void()> &callback);
    static void OnVMShutdown(const std::function<void()> &callback);
    static void OnUIRefresh(const std::function<void()> &callback);

  private:
    static std::vector<std::function<void()>> dvarinit_callbacks;
    static Detour Com_InitDvars_Detour;
    static void Com_InitDvars_Hook();

    static std::vector<std::function<void()>> cmdinit_callbacks;
    static Detour Cmd_Init_Detour;
    static void Cmd_Init_Hook();

    static std::vector<std::function<void()>> vmshutdown_callbacks;
    static Detour Scr_ShutdownSystem_Detour;
    static void *Scr_ShutdownSystem_Hook(scriptInstance_t inst, int sys, int bComplete);

    static std::vector<std::function<void()>> ui_refresh_callbacks;
    static Detour UI_Refresh_Detour;
    static int UI_Refresh_Hook(int localClientNum);
};
} // namespace mp
} // namespace t4
