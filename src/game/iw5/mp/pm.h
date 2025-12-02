#pragma once

#include "pch.h"

namespace iw5
{
namespace mp
{
class PlayerMovement : public Module
{
  public:
    PlayerMovement();
    ~PlayerMovement();

    const char *get_name() override
    {
        return "PlayerMovement";
    };
};
} // namespace mp
} // namespace iw5
