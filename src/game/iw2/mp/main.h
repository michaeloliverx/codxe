#pragma once

#include "plugin.h"

namespace iw2
{
namespace mp
{
class IW2_MP_Plugin : public Plugin
{

  public:
    IW2_MP_Plugin();
    ~IW2_MP_Plugin();
    static bool ShouldLoad();
};
} // namespace mp
} // namespace iw2
