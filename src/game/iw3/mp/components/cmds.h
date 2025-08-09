#pragma once

#include "common.h"

namespace iw3
{
namespace mp
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
} // namespace mp
} // namespace iw3
