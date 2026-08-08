#pragma once

#include "pch.h"

namespace t4
{
namespace sp
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
} // namespace sp
} // namespace t4
