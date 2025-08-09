#pragma once

#include "common.h"

namespace iw3
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
} // namespace iw3
