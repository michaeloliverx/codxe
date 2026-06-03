#pragma once

#include "pch.h"

namespace iw4
{

namespace mp
{
class pm : public Module
{
  public:
    pm();
    ~pm();

    const char *get_name() override
    {
        return "pm";
    }
};
} // namespace mp
} // namespace iw4
