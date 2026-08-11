#include "pch.h"
#include "brush_collision_tracker.h"

namespace
{
enum
{
    MAX_BRUSH_COUNT = USHRT_MAX + 1,
    BRUSHES_PER_WORD = sizeof(uint32_t) * 8,
};
uint32_t modifiedBrushes[MAX_BRUSH_COUNT / BRUSHES_PER_WORD];
} // namespace

namespace brush_collision_tracker
{
void Clear()
{
    memset(modifiedBrushes, 0, sizeof(modifiedBrushes));
}

void MarkModified(unsigned int index)
{
    modifiedBrushes[index / BRUSHES_PER_WORD] |= 1u << (index % BRUSHES_PER_WORD);
}

bool WasModified(unsigned int index)
{
    return (modifiedBrushes[index / BRUSHES_PER_WORD] & (1u << (index % BRUSHES_PER_WORD))) != 0;
}
} // namespace brush_collision_tracker
