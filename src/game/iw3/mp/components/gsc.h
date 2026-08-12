#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class GSC : public Module
{
  public:
    GSC();
    ~GSC();

    static void OnVMShutdown();

    const char *get_name() override
    {
        return "GSC";
    };
};
} // namespace mp
} // namespace iw3
