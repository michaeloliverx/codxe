#pragma once

#include <stdio.h>

namespace script_files
{
enum
{
    NO_FREE_HANDLES = -1,
    OPEN_FAILED = 0,
    MAX_HANDLES = 8,
};

bool IsValidHandle(int handle);
int Open(const char *path, const char *mode);
FILE *Get(int handle);
int Close(int handle);
void CloseAll();
} // namespace script_files
