#pragma once

namespace t4
{
namespace mp
{
class Debug : public Module
{
  public:
    Debug();
    ~Debug();

  private:
    static Detour ConsolePrintDetour;
    static void CL_ConsolePrint_Hook(int channel, const char *text, int flags);
};
} // namespace mp
} // namespace t4
