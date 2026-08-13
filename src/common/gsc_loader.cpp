#include "pch.h"
#include "gsc_loader.h"

namespace gsc_loader
{
char *TryLoadOverride(const char *scriptPath, AllocateTempMemory_t allocateTempMemory)
{
    if (!scriptPath || !allocateTempMemory)
        return nullptr;

    const std::string overridePath = Config::ResolveModPath(scriptPath);
    if (overridePath.empty())
        return nullptr;

    HANDLE file = CreateFileA(overridePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return nullptr;

    const DWORD fileSize = GetFileSize(file, nullptr);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0 || fileSize > static_cast<DWORD>(INT_MAX - 1))
    {
        CloseHandle(file);
        return nullptr;
    }

    char *buffer = static_cast<char *>(allocateTempMemory(static_cast<int>(fileSize + 1)));
    if (!buffer)
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

    DbgPrint("GSCLoader: Loaded override script: %s\n", overridePath.c_str());
    return buffer;
}

void DumpSource(const char *scriptPath, const char *contents)
{
    if (!scriptPath || !contents)
        return;

    const std::string dumpPath = filesystem::JoinPath(DUMP_DIR, scriptPath);
    if (dumpPath.empty())
        return;

    filesystem::CreateParentDirectories(dumpPath.c_str());

    HANDLE file =
        CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return;

    const DWORD contentsSize = static_cast<DWORD>(std::strlen(contents));
    DWORD bytesWritten = 0;
    const bool success =
        WriteFile(file, contents, contentsSize, &bytesWritten, nullptr) && bytesWritten == contentsSize;
    CloseHandle(file);

    if (success)
        DbgPrint("GSCLoader: Dumped script: %s\n", dumpPath.c_str());
}
} // namespace gsc_loader
