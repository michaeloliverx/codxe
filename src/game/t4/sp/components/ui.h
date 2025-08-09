#pragma once

#include "common.h"

namespace t4
{
namespace sp
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
} // namespace sp
} // namespace t4
