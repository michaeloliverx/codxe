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
    static void OnVMShutdown(const std::function<void()> &callback);

  private:
    static std::vector<std::function<void()>> dvarinit_callbacks;
    static Detour Com_InitDvars_Detour;
    static void Com_InitDvars_Hook();

    static std::vector<std::function<void()>> vmshutdown_callbacks;
    static Detour Scr_ShutdownSystem_Detour;
    static void *Scr_ShutdownSystem_Hook(scriptInstance_t inst, int sys, int bComplete);
};
} // namespace mp
} // namespace t4
