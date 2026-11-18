#pragma once
#include <xtl.h>

namespace codxe
{
// Join our fixed relative paths without a CRT dependency.
bool JoinPath(char out[MAX_PATH], const char *root, const char *relative);
bool GetModuleDirectory(char out[MAX_PATH], const WCHAR *path, unsigned length);
} // namespace codxe
