#pragma once

#include "pch.h"

namespace iw4
{
namespace sp
{
class GSC : public Module
{
  public:
    GSC();
    ~GSC();
    const char *get_name() override
    {
        return "GSC";
    };
};
} // namespace sp
} // namespace iw4
