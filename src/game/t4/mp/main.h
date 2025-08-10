#pragma once

namespace t4
{

namespace mp
{

class T4_MP_Plugin : public Plugin
{
  public:
    T4_MP_Plugin();
    ~T4_MP_Plugin();
    static bool ShouldLoad();
};

} // namespace mp
} // namespace t4
