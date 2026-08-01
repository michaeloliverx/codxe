#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
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
} // namespace mp_tu6
} // namespace iw4
