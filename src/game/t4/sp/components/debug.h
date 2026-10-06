#pragma once

namespace t4
{
namespace sp
{
class Debug : public Module
{
  public:
    Debug();
    ~Debug();

  private:
    static Detour ConsolePrintDetour;
    static void CL_ConsolePrint_Hook(int localClientNum, int channel, const char *txt, int duration, int pixelWidth,
                                     int flags);
};
} // namespace sp
} // namespace t4
