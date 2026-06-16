#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class assets : public Module
{
  public:
    assets();
    ~assets();

    const char *get_name() override
    {
        return "assets";
    };
};
} // namespace mp
} // namespace iw3
