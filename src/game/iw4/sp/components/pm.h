#pragma once

#include "common.h"

namespace iw4
{
namespace sp
{
class pm : public Module
{
  public:
    pm();
    ~pm();

    const char *get_name() override
    {
        return "pm";
    };
};
} // namespace sp
} // namespace iw4
