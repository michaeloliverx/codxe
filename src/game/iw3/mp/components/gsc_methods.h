#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class gsc_methods : public Module
{
  public:
    gsc_methods();
    ~gsc_methods();

    const char *get_name() override
    {
        return "gsc_methods";
    };
};
} // namespace mp
} // namespace iw3
