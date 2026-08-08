#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
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
} // namespace mp_tu6
} // namespace iw4
