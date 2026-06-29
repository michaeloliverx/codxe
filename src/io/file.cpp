#include "io/file.h"

#include "io/path.h"

#include <cstring>
#include <xtl.h>

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)
#endif

namespace codxe
{

bool FileExists(const char *path)
{
    if (path == 0 || path[0] == '\0')
        return false;

    const DWORD attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool CreateDirectories(const char *path)
{
    if (path == 0 || path[0] == '\0')
        return false;

    char temp[MAX_PATH];
    strncpy(temp, path, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    NormalizePathSlashes(temp);

    char *cursor = temp;
    char *device_root = strstr(temp, ":\\");
    if (device_root != 0)
        cursor = device_root + 2;

    for (; *cursor != '\0'; ++cursor)
    {
        if (*cursor == '\\')
        {
            *cursor = '\0';
            CreateDirectoryA(temp, 0);
            *cursor = '\\';
        }
    }

    return CreateDirectoryA(temp, 0) || GetLastError() == ERROR_ALREADY_EXISTS;
}

bool CreateDirectoriesForFile(const char *path)
{
    char parent[MAX_PATH];
    if (!GetParentPath(parent, sizeof(parent), path))
        return false;

    return CreateDirectories(parent);
}

bool ReadFileToBuffer(const char *path, void *buffer, size_t buffer_size, size_t *bytes_read)
{
    if (bytes_read != 0)
        *bytes_read = 0;

    if (path == 0 || buffer == 0 || buffer_size == 0)
        return false;

    HANDLE file =
        CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
        return false;

    const DWORD file_size = GetFileSize(file, 0);
    if (file_size == INVALID_FILE_SIZE || file_size > buffer_size)
    {
        CloseHandle(file);
        return false;
    }

    DWORD actual = 0;
    const BOOL ok = ReadFile(file, buffer, file_size, &actual, 0);
    CloseHandle(file);

    if (!ok || actual != file_size)
        return false;

    if (bytes_read != 0)
        *bytes_read = static_cast<size_t>(actual);

    return true;
}

bool WriteFileToDisk(const char *path, const void *data, size_t data_size)
{
    if (path == 0 || data == 0 || data_size == 0)
        return false;

    CreateDirectoriesForFile(path);

    HANDLE file = CreateFileA(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
        return false;

    DWORD actual = 0;
    const BOOL ok = WriteFile(file, data, static_cast<DWORD>(data_size), &actual, 0);
    CloseHandle(file);

    return ok && actual == data_size;
}

} // namespace codxe
