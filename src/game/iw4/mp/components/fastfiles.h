#pragma once

#include "pch.h"

namespace iw4
{
namespace mp
{
class fastfiles : public Module
{
  public:
    fastfiles();
    ~fastfiles();

    const char *get_name() override
    {
        return "fastfiles";
    }
};
} // namespace mp
} // namespace iw4
