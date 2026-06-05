#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class Patches : public Module
{
  public:
    Patches();

    const char *get_name() override
    {
        return "patches";
    }
};
} // namespace mp
} // namespace t4
