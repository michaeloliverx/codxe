#pragma once

#include <cstddef>
#include <xtl.h>

namespace codxe
{

void NormalizePathSlashes(char *path);
bool JoinPath(char *out_path, size_t out_path_size, const char *base_path, const char *relative_path);
bool GetParentPath(char *out_path, size_t out_path_size, const char *file_path);

} // namespace codxe
