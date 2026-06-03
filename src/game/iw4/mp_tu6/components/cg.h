#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
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
} // namespace mp_tu6
} // namespace iw4
