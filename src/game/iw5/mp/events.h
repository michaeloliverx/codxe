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

    const char *get_name() override
    {
        return "Events";
    };

    static void OnDvarInit(const std::function<void()> &callback);
    static void OnVMShutdown(const std::function<void(bool)> &callback);

  private:
    static std::vector<std::function<void()>> com_initdvars_callbacks;
    static std::vector<std::function<void(bool)>> vm_shutdown_callbacks;
    static Detour Com_InitDvars_Detour;
    static Detour G_ShutdownGame_Detour;
    static void Com_InitDvars_Hook();
    static void G_ShutdownGame_Hook(int freeScripts);
};
} // namespace mp
} // namespace iw5
