#pragma once

namespace t4
{
namespace sp
{
class SplitScreen : public Module
{
  public:
    SplitScreen();
    ~SplitScreen();

    static void OnDvarInit();
};
} // namespace sp
} // namespace t4
