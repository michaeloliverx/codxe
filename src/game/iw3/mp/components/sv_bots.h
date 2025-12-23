#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class sv_bots : public Module
{
  public:
    sv_bots();
    ~sv_bots();

    const char *get_name() override
    {
        return "sv_bots";
    };
};
} // namespace mp
} // namespace iw3
