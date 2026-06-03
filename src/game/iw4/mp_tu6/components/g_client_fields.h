#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class g_client_fields : public Module
{
  public:
    g_client_fields();
    ~g_client_fields();

    const char *get_name() override
    {
        return "g_client_fields";
    };
};
} // namespace mp_tu6
} // namespace iw4
