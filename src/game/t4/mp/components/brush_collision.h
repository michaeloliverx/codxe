#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class BrushCollision : public Module
{
  public:
    BrushCollision();
    ~BrushCollision();

    const char *get_name() override
    {
        return "BrushCollision";
    };
};
} // namespace mp
} // namespace t4
