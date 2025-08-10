#pragma once

namespace qos
{
namespace sp
{
class QOS_SP_Plugin : public Plugin
{
  public:
    QOS_SP_Plugin();
    static bool ShouldLoad();
};
} // namespace sp
} // namespace qos
