#pragma once

namespace iw2
{
namespace sp
{
class IW2_SP_Plugin : public Plugin
{

  public:
    IW2_SP_Plugin();
    ~IW2_SP_Plugin();
    static bool ShouldLoad();
};
} // namespace sp
} // namespace iw2
