#include "pch.h"
#include "gsc_loader.h"

namespace gsc_loader
{
char *TryLoadOverride(const char *scriptPath, AllocateTempMemory_t allocateTempMemory)
{
    const std::string overridePath = Config::ResolveModPath(scriptPath);
    return TryLoadOverride(scriptPath, overridePath.c_str(), allocateTempMemory);
}

char *TryLoadOverride(const char *scriptPath, const char *overridePath, AllocateTempMemory_t allocateTempMemory)
{
    if (!scriptPath || !overridePath || !*overridePath || !allocateTempMemory)
        return nullptr;

    HANDLE file = CreateFileA(overridePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
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

    DbgPrint("GSCLoader: Loaded override script: %s\n", overridePath);
    return buffer;
}
} // namespace gsc_loader
