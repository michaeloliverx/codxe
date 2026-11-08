#include "pch.h"
#include "debug_draw.h"

namespace t4
{
namespace mp
{
namespace
{
const unsigned char DEBUG_LINES_NO_DEPTH = 4;
const unsigned int GFXS0_CULL_NONE = 0x4000;
const unsigned int GFXS0_CULL_MASK = 0xC000;
const unsigned int GFXS1_DEPTHWRITE = 0x1;
const unsigned int GFXS1_DEPTHTEST_DISABLE = 0x2;
const unsigned int GFXS1_DEPTHTEST_MASK = 0xC;
const unsigned int GFXS1_POLYGON_OFFSET_MASK = 0x30;
const unsigned int GFXS1_STENCIL_MASK = 0xC0;

// Only the render backend changes this while executing a line command.
bool drawingNoDepthLines = false;
Material overlayLineMaterial;
GfxStateBits overlayLineStates[256];

const Material *OverlayLineMaterial(const Material *source)
{
    if (!source || !source->stateBitsTable || !source->stateBitsCount)
        return source;

    overlayLineMaterial = *source;
    memcpy(overlayLineStates, source->stateBitsTable, source->stateBitsCount * sizeof(GfxStateBits));
    for (unsigned int i = 0; i < source->stateBitsCount; ++i)
    {
        // Overlay quads are visible from either side and do not fight or modify world depth/stencil.
        overlayLineStates[i].loadBits[0] = (overlayLineStates[i].loadBits[0] & ~GFXS0_CULL_MASK) | GFXS0_CULL_NONE;
        overlayLineStates[i].loadBits[1] =
            (overlayLineStates[i].loadBits[1] &
             ~(GFXS1_DEPTHWRITE | GFXS1_DEPTHTEST_MASK | GFXS1_POLYGON_OFFSET_MASK | GFXS1_STENCIL_MASK)) |
            GFXS1_DEPTHTEST_DISABLE;
    }
    overlayLineMaterial.stateBitsTable = overlayLineStates;
    return &overlayLineMaterial;
}

void SetProjection(GfxCmdProjectionSet *cmd, GfxProjectionTypes projection)
{
    cmd->header.id = RC_PROJECTION_SET;
    cmd->header.byteCount = sizeof(*cmd);
    cmd->projection = projection;
}

// Reserve the draw and both projection changes together. A failed allocation submits nothing.
GfxCmdHeader *AllocateDraw(GfxRenderCommand id, int bytes)
{
    const int total = bytes + 2 * sizeof(GfxCmdProjectionSet);
    auto begin = reinterpret_cast<GfxCmdProjectionSet *>(R_GetCommandBuffer(RC_PROJECTION_SET, total));
    if (!begin)
        return nullptr;

    SetProjection(begin, GFX_PROJECTION_3D);
    auto draw = reinterpret_cast<GfxCmdHeader *>(begin + 1);
    draw->id = static_cast<unsigned short>(id);
    draw->byteCount = static_cast<unsigned short>(bytes);
    SetProjection(reinterpret_cast<GfxCmdProjectionSet *>(reinterpret_cast<unsigned char *>(draw) + bytes),
                  GFX_PROJECTION_2D);
    return draw;
}

int EntryCount(const void *entries, int num, int max, int capacity)
{
    if (!entries || num <= 0 || max != capacity)
        return 0;
    return num < capacity ? num : capacity;
}

void DrawLineQueue(const clientDebugLineInfo_t &info)
{
    const int count = EntryCount(info.lines, info.num, info.max, 2048);
    for (int i = 0; i < count;)
    {
        GfxPointVertex verts[128];
        const bool depthTest = info.lines[i].depthTest != 0;
        int batch = 0;
        while (i < count && batch < 64 && (info.lines[i].depthTest != 0) == depthTest)
        {
            const trDebugLine_t &line = info.lines[i++];
            GfxPointVertex *points = &verts[2 * batch++];
            memcpy(points[0].xyz, line.start, sizeof(line.start));
            memcpy(points[1].xyz, line.end, sizeof(line.end));
            DebugDraw::PackColor(line.color, points[0].color);
            memcpy(points[1].color, points[0].color, sizeof(points[0].color));
        }
        if (!DebugDraw::DrawLines(verts, batch, depthTest))
            return;
    }
}

void DrawStringQueue(const clientDebugStringInfo_t &info, const refdef_s &view)
{
    Font_s *font = *debugFont;
    if (!font)
        return;

    const int count = EntryCount(info.strings, info.num, info.max, 128);
    for (int i = 0; i < count; ++i)
    {
        const trDebugString_t &text = info.strings[i];
        size_t length = 0;
        while (length < sizeof(text.text) - 1 && text.text[length])
            ++length;
        const int bytes = (offsetof(GfxCmdDrawText3D, text) + static_cast<int>(length) + 4) & ~3;
        auto cmd = reinterpret_cast<GfxCmdDrawText3D *>(AllocateDraw(RC_DRAW_TEXT_3D, bytes));
        if (!cmd)
            return;

        memcpy(cmd->org, text.xyz, sizeof(cmd->org));
        cmd->font = font;
        for (int axis = 0; axis < 3; ++axis)
        {
            cmd->xPixelStep[axis] = -view.viewaxis[1][axis] * text.scale;
            cmd->yPixelStep[axis] = -view.viewaxis[2][axis] * text.scale;
        }
        DebugDraw::PackColor(text.color, cmd->color);
        memcpy(cmd->text, text.text, length);
        cmd->text[length] = '\0';
    }
}
} // namespace

void DebugDraw::PackColor(const float *color, unsigned char *packed)
{
    // Match TU7 R_ConvertColorToBytes: ARGB byte order on the big-endian renderer.
    for (int i = 0; i < 4; ++i)
    {
        const float value = color[i] > 0.0f ? (color[i] < 1.0f ? color[i] : 1.0f) : 0.0f;
        packed[(i + 1) & 3] = static_cast<unsigned char>(value * 255.0f + 0.5f);
    }
}

bool DebugDraw::DrawLines(const GfxPointVertex *verts, int lineCount, bool depthTest)
{
    if (!verts || lineCount <= 0 || lineCount > 64)
        return false;

    const int bytes = offsetof(GfxCmdDrawLines, verts) + 2 * lineCount * sizeof(GfxPointVertex);
    auto cmd = reinterpret_cast<GfxCmdDrawLines *>(AllocateDraw(RC_DRAW_LINES, bytes));
    if (!cmd)
        return false;
    cmd->lineCount = static_cast<short>(lineCount);
    cmd->width = 1;
    cmd->dimensions = depthTest ? 3 : DEBUG_LINES_NO_DEPTH;
    memcpy(cmd->verts, verts, 2 * lineCount * sizeof(GfxPointVertex));
    return true;
}

void DebugDraw::OnCGDrawActive(int localClientNum)
{
    if (localClientNum < 0 || localClientNum >= 4 || !*cgArray)
        return;

    // TU7 retains the queues and backend handlers, but strips the frontend debug draw submission.
    // The private line dimension selects an overlay material when depth testing is disabled.
    DrawLineQueue(clsDebug->svLinesBuffer);
    DrawLineQueue(clsDebug->clLines);
    const refdef_s &view = (*cgArray)[localClientNum].refdef;
    DrawStringQueue(clsDebug->svStringsBuffer, view);
    DrawStringQueue(clsDebug->clStrings, view);
}

Detour DebugDraw::DrawLinesDetour;
Detour DebugDraw::SetTessTechniqueDetour;

void DebugDraw::RB_DrawLinesCmd_Hook(GfxRenderCommandExecState *execState)
{
    const auto lines = static_cast<const GfxCmdDrawLines *>(execState->cmd);
    const bool previous = drawingNoDepthLines;
    drawingNoDepthLines = lines->dimensions == DEBUG_LINES_NO_DEPTH;
    // TU7 routes every non-2D dimension through its native 3D renderer.
    DrawLinesDetour.GetOriginal<decltype(RB_DrawLinesCmd)>()(execState);
    drawingNoDepthLines = previous;
}

void DebugDraw::RB_SetTessTechnique_Hook(const Material *material, MaterialTechniqueType techType)
{
    if (drawingNoDepthLines && techType == TECHNIQUE_UNLIT && material == *rgpLineMaterial)
        material = OverlayLineMaterial(material);
    SetTessTechniqueDetour.GetOriginal<decltype(RB_SetTessTechnique)>()(material, techType);
}

DebugDraw::DebugDraw()
{
    SetTessTechniqueDetour = Detour(RB_SetTessTechnique, RB_SetTessTechnique_Hook);
    SetTessTechniqueDetour.Install();
    DrawLinesDetour = Detour(RB_DrawLinesCmd, RB_DrawLinesCmd_Hook);
    DrawLinesDetour.Install();
}

DebugDraw::~DebugDraw()
{
    DrawLinesDetour.Remove();
    SetTessTechniqueDetour.Remove();
}
} // namespace mp
} // namespace t4
