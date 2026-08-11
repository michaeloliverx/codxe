#pragma once

namespace brush_collision_tracker
{
void Clear();
void MarkModified(unsigned int index);
bool WasModified(unsigned int index);
} // namespace brush_collision_tracker
