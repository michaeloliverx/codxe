#include "pch.h"
#include "gsc_loader.h"

namespace t4
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

    char dirPath[MAX_PATH];
    strncpy(dirPath, dumpPath.c_str(), sizeof(dirPath) - 1);
    dirPath[sizeof(dirPath) - 1] = '\0';

    char *lastSlash = strrchr(dirPath, '\\');
    if (lastSlash != nullptr)
    {
        *lastSlash = '\0';
        filesystem::create_nested_dirs(dirPath);
    }

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

char *GSCLoader::Scr_AddSourceBuffer_Hook(scriptInstance_t a1, const char *filename, const char *extFilename,
                                          const char *codePos, bool archive)
{
    auto callOriginal = [&]()
    {
        return Scr_AddSourceBuffer_Detour.GetOriginal<decltype(Scr_AddSourceBuffer)>()(a1, filename, extFilename,
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

GSCLoader::GSCLoader()
{
    Scr_AddSourceBuffer_Detour = Detour(Scr_AddSourceBuffer, GSCLoader::Scr_AddSourceBuffer_Hook);
    Scr_AddSourceBuffer_Detour.Install();
}

GSCLoader::~GSCLoader()
{
    Scr_AddSourceBuffer_Detour.Remove();
}
} // namespace mp
} // namespace t4
