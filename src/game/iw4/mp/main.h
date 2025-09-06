#pragma once

namespace iw4
{

namespace mp
{
class IW4_MP_Plugin : public Plugin
{

  public:
    IW4_MP_Plugin();
    static bool ShouldLoad();
};
} // namespace mp
} // namespace iw4
