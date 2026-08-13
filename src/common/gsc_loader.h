#pragma once

namespace gsc_loader
{
typedef void *(*AllocateTempMemory_t)(int size);

char *TryLoadOverride(const char *scriptPath, AllocateTempMemory_t allocateTempMemory);
void DumpSource(const char *scriptPath, const char *contents);
} // namespace gsc_loader
