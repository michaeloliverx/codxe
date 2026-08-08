#pragma once

#include "pch.h"

namespace iw4
{
namespace sp
{
class g_scr_main : public Module
{
  public:
    g_scr_main();
    ~g_scr_main();
    const char *get_name() override
    {
        return "g_scr_main";
    };
};
} // namespace sp
} // namespace iw4
