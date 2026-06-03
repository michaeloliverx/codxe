#pragma once

#include "pch.h"

namespace iw3
{
namespace sp
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

    static void OnCmdInit(const std::function<void()> &callback);
    static void OnDvarInit(const std::function<void()> &callback);

  private:
    static std::vector<std::function<void()>> cmdinit_callbacks;
    static Detour Cmd_Init_Detour;
    static void Cmd_Init_Hook();

    static std::vector<std::function<void()>> dvarinit_callbacks;
    static Detour Com_InitDvars_Detour;
    static void Com_InitDvars_Hook();
};
} // namespace sp
} // namespace iw3
