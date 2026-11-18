#pragma once

namespace gsc_loader
{
typedef void *(*AllocateTempMemory_t)(int size);

char *TryLoadOverride(const char *scriptPath, AllocateTempMemory_t allocateTempMemory);
char *TryLoadOverride(const char *scriptPath, const char *overridePath, AllocateTempMemory_t allocateTempMemory);
} // namespace gsc_loader
