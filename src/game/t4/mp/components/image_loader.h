#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class image_loader : public Module
{
  public:
    image_loader();
    ~image_loader();

    const char *get_name() override
    {
        return "image_loader";
    };
};
} // namespace mp
} // namespace t4
