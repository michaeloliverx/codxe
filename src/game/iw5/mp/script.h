#pragma once

#include "common.h"

namespace iw5
{
namespace mp
{
class Script : public Module
{
  public:
    Script();
    ~Script();

    const char *get_name() override
    {
        return "Script";
    };
};
} // namespace mp
} // namespace iw5
