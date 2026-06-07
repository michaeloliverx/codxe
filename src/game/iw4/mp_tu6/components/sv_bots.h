#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class SVBots : public Module
{
  public:
    SVBots();
    ~SVBots();

    const char *get_name() override
    {
        return "SVBots";
    };
};
} // namespace mp_tu6
} // namespace iw4
