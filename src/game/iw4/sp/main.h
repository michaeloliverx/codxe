#pragma once

namespace iw4
{

namespace sp
{
class IW4_SP_Plugin : public Plugin
{

  public:
    IW4_SP_Plugin();
    static bool ShouldLoad();
};
} // namespace sp
} // namespace iw4
