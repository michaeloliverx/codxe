#pragma once

#include "pch.h"

namespace iw3
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
} // namespace iw3
