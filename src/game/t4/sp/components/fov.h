#pragma once

namespace t4
{
namespace sp
{
class Fov : public Module
{
  public:
    Fov();
    ~Fov();
    static void OnDvarInit();
};
} // namespace sp
} // namespace t4
