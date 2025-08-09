#pragma once

#include "common.h"

namespace t4
{
namespace mp
{
class ui : public Module
{
  public:
    ui();
    ~ui();

    const char *get_name() override
    {
        return "ui";
    };
};
} // namespace mp
} // namespace t4
