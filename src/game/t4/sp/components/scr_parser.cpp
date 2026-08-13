#include "pch.h"
#include "scr_parser.h"

namespace t4
{
namespace sp
{
namespace
{
void NormalizePath(char *path)
{
    if (path == nullptr)
        return;

    for (char *cursor = path; *cursor != '\0'; ++cursor)
    {
        if (*cursor == '/')
            *cursor = '\\';
    }
}

bool BuildScriptPath(char *path, size_t pathSize, const char *basePath, const char *scriptPath)
{
    if (path == nullptr || pathSize == 0 || basePath == nullptr || basePath[0] == '\0' || scriptPath == nullptr ||
        scriptPath[0] == '\0')
    {
        return false;
    }

    const int written = _snprintf_s(path, pathSize, _TRUNCATE, "%s\\%s", basePath, scriptPath);
    path[pathSize - 1] = '\0';

    if (written < 0 || static_cast<size_t>(written) >= pathSize)
        return false;

    NormalizePath(path);
    return true;
}

char *ReadFileToGameTempBuffer(const char *path)
{
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return nullptr;

    const DWORD fileSize = GetFileSize(file, nullptr);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0)
    {
        CloseHandle(file);
        return nullptr;
    }

    char *buffer = static_cast<char *>(Hunk_AllocateTempMemoryHighInternal(fileSize + 1));
    if (buffer == nullptr)
    {
        CloseHandle(file);
        return nullptr;
    }

    DWORD bytesRead = 0;
    if (!ReadFile(file, buffer, fileSize, &bytesRead, nullptr) || bytesRead != fileSize)
    {
        CloseHandle(file);
        return nullptr;
    }

    CloseHandle(file);
    buffer[fileSize] = '\0';
    return buffer;
}

void WriteScriptDump(const char *scriptPath, const char *contents)
{
    if (scriptPath == nullptr || contents == nullptr)
        return;

    char dumpPath[MAX_PATH];
    if (!BuildScriptPath(dumpPath, sizeof(dumpPath), DUMP_DIR, scriptPath))
        return;

    char dirPath[MAX_PATH];
    strncpy(dirPath, dumpPath, sizeof(dirPath) - 1);
    dirPath[sizeof(dirPath) - 1] = '\0';

    char *lastSlash = strrchr(dirPath, '\\');
    if (lastSlash != nullptr)
    {
        *lastSlash = '\0';
        filesystem::create_nested_dirs(dirPath);
    }

    HANDLE file = CreateFileA(dumpPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return;

    DWORD bytesWritten = 0;
    const DWORD contentSize = static_cast<DWORD>(std::strlen(contents));
    WriteFile(file, contents, contentSize, &bytesWritten, nullptr);
    CloseHandle(file);
}
} // namespace

Detour Scr_AddSourceBuffer_Detour;

char *Scr_AddSourceBuffer_Hook(scriptInstance_t inst, const char *filename, const char *extFilename,
                               const char *codePos, bool archive)
{
    auto callOriginal = [&]()
    {
        return Scr_AddSourceBuffer_Detour.GetOriginal<decltype(Scr_AddSourceBuffer)>()(inst, filename, extFilename,
                                                                                       codePos, archive);
    };

    if (Config::dump_rawfile)
    {
        char *contents = callOriginal();
        if (contents != nullptr)
            WriteScriptDump(extFilename, contents);

        return contents;
    }

    char overridePath[MAX_PATH];
    if (!BuildScriptPath(overridePath, sizeof(overridePath), Config::GetModBasePath(), extFilename))
        return callOriginal();

    char *buffer = ReadFileToGameTempBuffer(overridePath);
    if (buffer == nullptr)
        return callOriginal();

    DbgPrint("GSCLoader: Loaded override script: %s\n", overridePath);
    return buffer;
}

scr_parser::scr_parser()
{
    Scr_AddSourceBuffer_Detour = Detour(Scr_AddSourceBuffer, Scr_AddSourceBuffer_Hook);
    Scr_AddSourceBuffer_Detour.Install();
}

scr_parser::~scr_parser()
{
    Scr_AddSourceBuffer_Detour.Remove();
}
} // namespace sp
} // namespace t4
