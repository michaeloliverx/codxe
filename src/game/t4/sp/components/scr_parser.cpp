#include "pch.h"
#include "scr_parser.h"

namespace t4
{
namespace sp
{
namespace
{
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

    const std::string dumpPath = filesystem::JoinPath(DUMP_DIR, scriptPath);
    if (dumpPath.empty())
        return;

    filesystem::CreateParentDirectories(dumpPath.c_str());

    HANDLE file =
        CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
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
} // namespace sp
} // namespace t4
