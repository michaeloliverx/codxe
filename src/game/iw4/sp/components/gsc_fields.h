#pragma once

#include "pch.h"

namespace iw4
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
} // namespace iw4
