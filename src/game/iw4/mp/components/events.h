#pragma once

#include "pch.h"

namespace iw4
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
    }

    static void OnDvarInit(const std::function<void()> &callback);

  private:
    static std::vector<std::function<void()>> com_initdvars_callbacks;
    static Detour Com_InitDvars_Detour;
    static void Com_InitDvars_Hook();
};
} // namespace mp
} // namespace iw4
