#include "pch.h"
#include "scr_parser.h"

namespace iw3
{
namespace mp
{
namespace
{
const size_t MAX_SCRIPT_PATH = 512;

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

bool BuildScriptPath(char *path, size_t path_size, const char *base_path, const char *script_path)
{
    if (path == nullptr || path_size == 0 || base_path == nullptr || base_path[0] == '\0' || script_path == nullptr ||
        script_path[0] == '\0')
    {
        return false;
    }

    const int written = _snprintf_s(path, path_size, _TRUNCATE, "%s\\%s", base_path, script_path);
    path[path_size - 1] = '\0';

    if (written < 0 || static_cast<size_t>(written) >= path_size)
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

    const DWORD file_size = GetFileSize(file, nullptr);
    if (file_size == INVALID_FILE_SIZE || file_size == 0)
    {
        CloseHandle(file);
        return nullptr;
    }

    char *buffer = static_cast<char *>(Hunk_AllocateTempMemoryHighInternal(file_size + 1));
    if (buffer == nullptr)
    {
        CloseHandle(file);
        return nullptr;
    }

    DWORD bytes_read = 0;
    if (!ReadFile(file, buffer, file_size, &bytes_read, nullptr) || bytes_read != file_size)
    {
        CloseHandle(file);
        return nullptr;
    }

    CloseHandle(file);
    buffer[file_size] = '\0';
    return buffer;
}

void WriteScriptDump(const char *script_path, const char *contents)
{
    if (script_path == nullptr || contents == nullptr)
        return;

    char dump_path[MAX_SCRIPT_PATH];
    if (!BuildScriptPath(dump_path, sizeof(dump_path), DUMP_DIR, script_path))
        return;

    char dir_path[MAX_SCRIPT_PATH];
    strncpy(dir_path, dump_path, sizeof(dir_path) - 1);
    dir_path[sizeof(dir_path) - 1] = '\0';

    char *last_slash = strrchr(dir_path, '\\');
    if (last_slash != nullptr)
    {
        *last_slash = '\0';
        filesystem::create_nested_dirs(dir_path);
    }

    HANDLE file = CreateFileA(dump_path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return;

    DWORD bytes_written = 0;
    const DWORD content_size = static_cast<DWORD>(std::strlen(contents));
    WriteFile(file, contents, content_size, &bytes_written, nullptr);
    CloseHandle(file);
}
} // namespace

Detour Scr_AddSourceBuffer_Detour;

char *Scr_AddSourceBuffer_Hook(const char *filename, const char *extFilename, const char *codePos, bool archive)
{
    auto callOriginal = [&]()
    {
        return Scr_AddSourceBuffer_Detour.GetOriginal<decltype(Scr_AddSourceBuffer)>()(filename, extFilename, codePos,
                                                                                       archive);
    };

    if (Config::dump_rawfile)
    {
        char *contents = callOriginal();
        if (contents != nullptr)
            WriteScriptDump(extFilename, contents);

        return contents;
    }

    char override_path[MAX_SCRIPT_PATH];
    if (!BuildScriptPath(override_path, sizeof(override_path), Config::GetModBasePathCStr(), extFilename))
        return callOriginal();

    char *buffer = ReadFileToGameTempBuffer(override_path);
    if (buffer == nullptr)
        return callOriginal();

    DbgPrint("GSCLoader: Loaded override script: %s\n", override_path);
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
} // namespace mp
} // namespace iw3
