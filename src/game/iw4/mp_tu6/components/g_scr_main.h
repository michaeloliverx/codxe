#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
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
} // namespace mp_tu6
} // namespace iw4
