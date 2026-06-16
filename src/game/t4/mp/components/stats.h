#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class stats : public Module
{
  public:
    stats();
    ~stats();

    const char *get_name() override
    {
        return "stats";
    }
};
} // namespace mp
} // namespace t4
