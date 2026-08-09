#pragma once

#include "pch.h"

namespace iw3
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
} // namespace iw3
