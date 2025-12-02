#pragma once

#include "pch.h"

namespace iw4
{
namespace mp
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
} // namespace mp
} // namespace iw4
