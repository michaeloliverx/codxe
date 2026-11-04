#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class DebugDraw : public Module
{
  public:
    DebugDraw();
    ~DebugDraw();

    static void OnCGDrawActive(int localClientNum);

  private:
    static Detour DrawLinesDetour;
    static void RB_DrawLinesCmd_Hook(GfxRenderCommandExecState *execState);
};
} // namespace mp_tu6
} // namespace iw4
