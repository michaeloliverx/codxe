#include "pch.h"
#include "gsc_files.h"
#include "common/script_files.h"

namespace iw2
{
namespace mp
{
namespace
{
std::vector<std::string> fileArgs[script_files::MAX_HANDLES];
char fileModes[script_files::MAX_HANDLES];

int GetHandle(unsigned int param)
{
    const int handle = Scr_GetInt(param);
    if (handle < 0 || handle >= script_files::MAX_HANDLES || !script_files::Get(handle + 1))
    {
        Scr_Error("invalid or closed script file handle");
        return -1;
    }
    return handle;
}
} // namespace

void CloseScriptFiles()
{
    script_files::CloseAll();
    for (int i = 0; i < script_files::MAX_HANDLES; ++i)
    {
        fileArgs[i].clear();
        fileModes[i] = 0;
    }
}

void GScr_OpenFile()
{
    if (Scr_GetNumParam() != 2)
        Scr_Error("Usage: openfile(<filename>, <read/write/append>);");
    const char *filename = Scr_GetString(0);
    const char *mode = Scr_GetString(1);
    const char *fileMode = nullptr;
    if (!_stricmp(mode, "read"))
        fileMode = "rb";
    else if (!_stricmp(mode, "write"))
        fileMode = "wb";
    else if (!_stricmp(mode, "append"))
        fileMode = "ab";
    else
    {
        Scr_Error("openfile mode must be read, write, or append");
        return;
    }
    if (!*filename || filename[0] == '/' || filename[0] == '\\' || strchr(filename, ':') || strstr(filename, ".."))
    {
        Scr_Error("openfile requires a path relative to scriptdata");
        return;
    }
    const std::string path = Config::ResolveModPath((std::string("scriptdata/") + filename).c_str());
    if (fileMode[0] != 'r')
        filesystem::CreateParentDirectories(path.c_str());
    const int handle = script_files::Open(path.c_str(), fileMode);
    if (handle > 0)
    {
        fileArgs[handle - 1].clear();
        fileModes[handle - 1] = fileMode[0];
    }
    // IW2 exposes zero-based handles, with -1 for an unsuccessful open.
    Scr_AddInt(handle > 0 ? handle - 1 : -1);
}

void GScr_CloseFile()
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: closefile(<file>);");
    const int handle = GetHandle(0);
    if (handle < 0)
        return;
    script_files::Close(handle + 1);
    fileArgs[handle].clear();
    fileModes[handle] = 0;
}

void GScr_FReadLn()
{
    if (Scr_GetNumParam() != 1)
        Scr_Error("Usage: freadln(<file>);");
    const int handle = GetHandle(0);
    if (handle < 0)
        return;
    if (fileModes[handle] != 'r')
        Scr_Error("freadln requires a file opened for reading");
    std::vector<std::string> &args = fileArgs[handle];
    args.clear();
    char buffer[8192];
    FILE *file = script_files::Get(handle + 1);
    if (!fgets(buffer, sizeof(buffer), file))
    {
        Scr_AddInt(-1);
        return;
    }
    size_t len = strlen(buffer);
    if (len == sizeof(buffer) - 1 && buffer[len - 1] != '\n' && !feof(file))
        Scr_Error("freadln: line exceeds 8191 bytes");
    while (len && (buffer[len - 1] == '\r' || buffer[len - 1] == '\n'))
        buffer[--len] = 0;
    // Preserve empty CSV columns used by Bot Warfare's optional waypoint angles.
    char *start = buffer;
    for (char *p = buffer;; ++p)
    {
        if (*p == ',' || !*p)
        {
            const bool end = !*p;
            args.push_back(std::string(start, p - start));
            if (end)
                break;
            start = p + 1;
        }
    }
    Scr_AddInt(static_cast<int>(args.size()));
}

void GScr_FGetArg()
{
    if (Scr_GetNumParam() != 2)
        Scr_Error("Usage: fgetarg(<file>, <column>);");
    const int handle = GetHandle(0);
    if (handle < 0)
        return;
    const int column = Scr_GetInt(1);
    if (column < 0 || static_cast<size_t>(column) >= fileArgs[handle].size())
        Scr_Error("fgetarg: column is outside the last line read");
    Scr_AddString(fileArgs[handle][column].c_str());
}

void GScr_FPrintLn()
{
    if (Scr_GetNumParam() != 2)
        Scr_Error("Usage: fprintln(<file>, <text>);");
    const int handle = GetHandle(0);
    if (handle < 0)
        return;
    if (fileModes[handle] == 'r')
        Scr_Error("fprintln requires a file opened for writing");
    const char *text = Scr_GetString(1);
    // The editor supplies newlines and CSV delimiters itself.
    const size_t len = strlen(text);
    Scr_AddBool(fwrite(text, 1, len, script_files::Get(handle + 1)) == len);
}
} // namespace mp
} // namespace iw2
