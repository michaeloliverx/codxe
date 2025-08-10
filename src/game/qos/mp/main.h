#pragma once

namespace qos
{
namespace mp
{
class QOS_MP_Plugin : public Plugin
{

  public:
    QOS_MP_Plugin();
    static bool ShouldLoad();
};
} // namespace mp
} // namespace qos
