#include "paths.h"

bool codxe::GetModuleDirectory(char out[MAX_PATH], const WCHAR *path, unsigned length)
{
    out[0] = 0;
    if (!path || length >= MAX_PATH)
        return false;
    unsigned lastSlash = 0;
    for (unsigned i = 0; i < length; ++i)
    {
        if (!path[i] || path[i] > 127)
            return false;
        out[i] = static_cast<char>(path[i]);
        if (path[i] == '\\')
            lastSlash = i;
    }
    out[lastSlash] = 0;
    return lastSlash != 0;
}

bool codxe::JoinPath(char out[MAX_PATH], const char *root, const char *relative)
{
    if (!root || !*root || !relative || !*relative)
        return false;

    unsigned used = 0;
    const char *parts[] = {root, relative};
    for (unsigned part = 0; part < 2; ++part)
    {
        if (part && out[used - 1] != '\\')
        {
            if (used == MAX_PATH - 1)
                return false;
            out[used++] = '\\';
        }
        for (const char *text = parts[part]; *text; ++text)
        {
            if (used == MAX_PATH - 1)
            {
                out[0] = 0;
                return false;
            }
            out[used++] = *text == '/' ? '\\' : *text;
        }
    }
    out[used] = 0;
    return true;
}
