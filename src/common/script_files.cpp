#include "pch.h"
#include "script_files.h"

namespace
{
FILE *handles[script_files::MAX_HANDLES];
} // namespace

namespace script_files
{
bool IsValidHandle(int handle)
{
    return handle > 0 && handle <= MAX_HANDLES;
}

int Open(const char *path, const char *mode)
{
    for (int i = 0; i < MAX_HANDLES; ++i)
    {
        if (!handles[i])
        {
            handles[i] = fopen(path, mode);
            return handles[i] ? i + 1 : OPEN_FAILED;
        }
    }

    return NO_FREE_HANDLES;
}

FILE *Get(int handle)
{
    return IsValidHandle(handle) ? handles[handle - 1] : nullptr;
}

int Close(int handle)
{
    if (!IsValidHandle(handle) || !handles[handle - 1])
        return EOF;

    FILE *file = handles[handle - 1];
    handles[handle - 1] = nullptr;
    return fclose(file);
}

void CloseAll()
{
    for (int handle = 1; handle <= MAX_HANDLES; ++handle)
    {
        if (Get(handle))
            Close(handle);
    }
}
} // namespace script_files
