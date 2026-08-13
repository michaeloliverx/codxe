#include "pch.h"
#include "scr_parser.h"

namespace iw3
{
namespace mp
{
namespace
{
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

    const std::string dumpPath = filesystem::JoinPath(DUMP_DIR, script_path);
    if (dumpPath.empty())
        return;

    filesystem::CreateParentDirectories(dumpPath.c_str());

    HANDLE file =
        CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
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

    const char *modBasePath = Config::GetModBasePath();
    if (modBasePath == nullptr || modBasePath[0] == '\0')
        return callOriginal();

    const std::string overridePath = filesystem::JoinPath(modBasePath, extFilename);
    char *buffer = ReadFileToGameTempBuffer(overridePath.c_str());
    if (buffer == nullptr)
        return callOriginal();

    DbgPrint("GSCLoader: Loaded override script: %s\n", overridePath.c_str());
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
