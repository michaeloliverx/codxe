#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class GSCClientMethods : public Module
{
  public:
    GSCClientMethods();
    ~GSCClientMethods();

    const char *get_name() override
    {
        return "GSCClientMethods";
    };
};
} // namespace mp
} // namespace t4
