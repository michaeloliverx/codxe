#pragma once

#include "pch.h"

namespace iw5
{
namespace mp
{
// Retail MW3 has the interactive console compiled out (no toggleconsole command, no console drawing),
// so this is a small overlay: keyboard input via XInputGetKeystroke, commands via Cbuf_AddText,
// output captured from CL_ConsolePrint and drawn with UI_DrawText.
class Console : public Module
{
  public:
    Console();
    ~Console();

  private:
    static Detour UI_Refresh_Detour;
    static void UI_Refresh_Hook(LocalClientNum_t localClientNum);

    static Detour CL_ConsolePrint_Detour;
    static void CL_ConsolePrint_Hook(LocalClientNum_t localClientNum, int channel, const char *txt,
                                     unsigned int duration, unsigned int pixelWidth, int flags);
};
} // namespace mp
} // namespace iw5
