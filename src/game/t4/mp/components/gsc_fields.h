#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class GSCFields : public Module
{
  public:
    GSCFields();
    ~GSCFields();

    const char *get_name() override
    {
        return "GSCFields";
    };
};
} // namespace mp
} // namespace t4
