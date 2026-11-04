#include "pch.h"
#include "debug_draw.h"

namespace iw4
{
namespace mp_tu6
{
namespace
{
// Native 3D line commands always enable depth testing.
const unsigned char DEBUG_LINES_NO_DEPTH = 4;

bool SetDebugProjection(GfxProjectionTypes projection)
{
    auto command =
        reinterpret_cast<GfxCmdProjectionSet *>(R_GetCommandBuffer(RC_PROJECTION_SET, sizeof(GfxCmdProjectionSet)));
    if (!command)
        return false;
    command->projection = projection;
    return true;
}

int DebugEntryCount(const void *entries, int num, int max, int capacity)
{
    if (!entries || num <= 0 || max != capacity)
        return 0;
    return num < capacity ? num : capacity;
}
} // namespace

Detour DebugDraw::DrawLinesDetour;

void DebugDraw::RB_DrawLinesCmd_Hook(GfxRenderCommandExecState *execState)
{
    const auto lines = static_cast<const GfxCmdDrawLines *>(execState->cmd);
    if (lines->dimensions == DEBUG_LINES_NO_DEPTH)
    {
        RB_DrawLines3D(lines->lineCount, lines->width, lines->verts, false);
        execState->cmd = static_cast<const unsigned char *>(execState->cmd) + lines->header.byteCount;
        return;
    }

    DrawLinesDetour.GetOriginal<decltype(RB_DrawLinesCmd)>()(execState);
}

void DebugDraw::OnCGDrawActive(int localClientNum)
{
    // TU6's CL_UpdateDebugClientData is empty, so submit the server queues here.
    const clientDebugLineInfo_t &lines = clsDebug->svLinesBuffer;
    const clientDebugStringInfo_t &strings = clsDebug->svStringsBuffer;
    const int lineCount = DebugEntryCount(lines.lines, lines.num, lines.max, 2048);
    const int stringCount = DebugEntryCount(strings.strings, strings.num, strings.max, 128);
    if ((!lineCount && !stringCount) || !*cgArray || !sharedUiInfo->assets.consoleFont)
        return;

    if (!SetDebugProjection(GFX_PROJECTION_3D))
        return;

    const auto &viewAxis = (*cgArray)[localClientNum].refdef.view.axis;

    for (int i = 0; i < lineCount;)
    {
        GfxPointVertex verts[128];
        const bool depthTest = lines.lines[i].depthTest != 0;
        int batch = 0;
        while (i < lineCount && batch < 64 && (lines.lines[i].depthTest != 0) == depthTest)
        {
            const trDebugLine_t &line = lines.lines[i++];
            GfxPointVertex *points = &verts[2 * batch++];
            memcpy(points[0].xyz, line.start, sizeof(line.start));
            memcpy(points[1].xyz, line.end, sizeof(line.end));
            R_ConvertColorToBytes(line.color, points[0].color);
            memcpy(points[1].color, points[0].color, sizeof(points[0].color));
        }
        R_AddCmdDrawLines(static_cast<short>(batch), 1, depthTest ? 3 : DEBUG_LINES_NO_DEPTH, verts);
    }

    for (int i = 0; i < stringCount; ++i)
    {
        const trDebugString_t &text = strings.strings[i];
        float xPixelStep[3];
        float yPixelStep[3];
        for (int axis = 0; axis < 3; ++axis)
        {
            xPixelStep[axis] = -viewAxis[1][axis] * text.scale;
            yPixelStep[axis] = -viewAxis[2][axis] * text.scale;
        }
        R_AddCmdDrawTextInSpace(text.text, sharedUiInfo->assets.consoleFont, text.xyz, xPixelStep, yPixelStep,
                                text.color);
    }

    SetDebugProjection(GFX_PROJECTION_2D);
}

DebugDraw::DebugDraw()
{
    DrawLinesDetour = Detour(RB_DrawLinesCmd, RB_DrawLinesCmd_Hook);
    DrawLinesDetour.Install();
}

DebugDraw::~DebugDraw()
{
    DrawLinesDetour.Remove();
}
} // namespace mp_tu6
} // namespace iw4
