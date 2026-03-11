#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class gsc_hud_elem : public Module
{
  public:
    gsc_hud_elem();
    ~gsc_hud_elem();

    const char *get_name() override
    {
        return "gsc_hud_elem";
    };
};
} // namespace mp
} // namespace iw3
