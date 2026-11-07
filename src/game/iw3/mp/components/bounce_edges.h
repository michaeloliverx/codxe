#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
namespace bounce_edges
{
void RegisterDvars();
void Reset();
void Draw(int localClientNum, const std::vector<unsigned int> &worldTerrain, float drawDistance);
} // namespace bounce_edges
} // namespace mp
} // namespace iw3
