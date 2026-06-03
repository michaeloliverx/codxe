#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class patches : public Module
{
  public:
    patches();
    ~patches();

    const char *get_name() override
    {
        return "patches";
    };
};
} // namespace mp_tu6
} // namespace iw4
