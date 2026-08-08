#pragma once

#include "pch.h"

namespace qos
{
namespace mp
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
} // namespace mp
} // namespace qos
