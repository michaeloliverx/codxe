#pragma once

#include "common.h"

namespace iw3
{
namespace mp
{
class IW3_MP_Plugin : public Plugin
{

  public:
    IW3_MP_Plugin();
    ~IW3_MP_Plugin();
    static bool ShouldLoad();
};
} // namespace mp
} // namespace iw3
