#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class DebugDraw : public Module
{
  public:
    DebugDraw();
    ~DebugDraw();

    static void OnCGDrawActive(int localClientNum);

    // Each submission includes both projection commands, even if the command buffer fills up.
    static bool DrawLines(const GfxPointVertex *verts, int lineCount, bool depthTest = false);
    static void PackColor(const float *color, unsigned char *packed);

  private:
    static Detour DrawLinesDetour;
    static Detour SetTessTechniqueDetour;
    static void RB_DrawLinesCmd_Hook(GfxRenderCommandExecState *execState);
    static void RB_SetTessTechnique_Hook(const Material *material, MaterialTechniqueType techType);
};
} // namespace mp
} // namespace iw3
