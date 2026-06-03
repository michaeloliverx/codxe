#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class scr_parser : public Module
{
  public:
    scr_parser();
    ~scr_parser();

    const char *get_name() override
    {
        return "scr_parser";
    };
};
} // namespace mp_tu6
} // namespace iw4
