#include "io/path.h"

#include <cstdio>
#include <cstring>

namespace codxe
{

void NormalizePathSlashes(char *path)
{
    if (path == 0)
        return;

    for (char *cursor = path; *cursor != '\0'; ++cursor)
    {
        if (*cursor == '/')
            *cursor = '\\';
    }
}

bool JoinPath(char *out_path, size_t out_path_size, const char *base_path, const char *relative_path)
{
    if (out_path == 0 || out_path_size == 0 || base_path == 0 || base_path[0] == '\0' || relative_path == 0 ||
        relative_path[0] == '\0')
    {
        return false;
    }

    const int written = _snprintf_s(out_path, out_path_size, _TRUNCATE, "%s\\%s", base_path, relative_path);
    out_path[out_path_size - 1] = '\0';
    NormalizePathSlashes(out_path);

    return written >= 0 && static_cast<size_t>(written) < out_path_size;
}

bool GetParentPath(char *out_path, size_t out_path_size, const char *file_path)
{
    if (out_path == 0 || out_path_size == 0 || file_path == 0 || file_path[0] == '\0')
        return false;

    strncpy(out_path, file_path, out_path_size - 1);
    out_path[out_path_size - 1] = '\0';
    NormalizePathSlashes(out_path);

    char *last_slash = strrchr(out_path, '\\');
    if (last_slash == 0)
        return false;

    *last_slash = '\0';
    return out_path[0] != '\0';
}

} // namespace codxe
