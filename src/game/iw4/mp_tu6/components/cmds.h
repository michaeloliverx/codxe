#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class cmds : public Module
{
  public:
    cmds();
    ~cmds();

    const char *get_name() override
    {
        return "cmds";
    };
};
} // namespace mp_tu6
} // namespace iw4
