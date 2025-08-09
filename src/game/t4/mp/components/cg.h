#pragma once

#include "common.h"

namespace t4
{
namespace mp
{
class cg : public Module
{
  public:
    cg();
    ~cg();

    const char *get_name() override
    {
        return "cg";
    };
};
} // namespace mp
} // namespace t4
