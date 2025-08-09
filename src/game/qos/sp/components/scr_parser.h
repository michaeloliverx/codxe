#pragma once

#include "common.h"

namespace qos
{
namespace sp
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
} // namespace sp
} // namespace qos
