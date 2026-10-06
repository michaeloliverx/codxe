#pragma once

namespace iw3
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
    static void CL_ConsolePrint_Hook(int localClientNum, int channel, const char *txt, int duration, int pixelWidth,
                                     int flags);
};
} // namespace mp
} // namespace iw3
