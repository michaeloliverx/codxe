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

  private:
    static Detour Cmd_Init_Detour;
    static void Cmd_Init_Hook();
};
} // namespace sp
} // namespace iw3
