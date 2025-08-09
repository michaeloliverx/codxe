#pragma once

#include "common.h"

namespace t4
{
namespace sp
{
class g_client_script_cmd : public Module
{
  public:
    g_client_script_cmd();
    ~g_client_script_cmd();
    const char *get_name() override
    {
        return "g_client_script_cmd";
    };
};
} // namespace sp
} // namespace t4
