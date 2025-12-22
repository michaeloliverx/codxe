#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class gsc_client_fields : public Module
{
  public:
    gsc_client_fields();
    ~gsc_client_fields();

    const char *get_name() override
    {
        return "gsc_client_fields";
    };
};
} // namespace mp
} // namespace iw3
