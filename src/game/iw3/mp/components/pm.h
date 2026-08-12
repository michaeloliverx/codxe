#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class pm : public Module
{
  public:
    pm();
    ~pm();

    static void OnDvarInit();
    static void OnCGDrawActive();

    const char *get_name() override
    {
        return "pm";
    };
};
} // namespace mp
} // namespace iw3
