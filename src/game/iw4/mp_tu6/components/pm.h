#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
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
} // namespace mp_tu6
} // namespace iw4
