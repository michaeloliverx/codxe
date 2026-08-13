#include "pch.h"

namespace filesystem
{
void NormalizePathSeparators(char *path)
{
    if (!path)
        return;

    for (; *path; ++path)
    {
        if (*path == '/')
            *path = '\\';
    }
}

std::string JoinPath(const char *basePath, const char *relativePath)
{
    std::string path = basePath ? basePath : "";
    const char *relative = relativePath ? relativePath : "";

    const bool baseHasSeparator = !path.empty() && (path[path.size() - 1] == '\\' || path[path.size() - 1] == '/');
    const bool relativeHasSeparator = relative[0] == '\\' || relative[0] == '/';

    if (!path.empty() && relative[0] != '\0' && !baseHasSeparator && !relativeHasSeparator)
        path += '\\';
    else if (baseHasSeparator && relativeHasSeparator)
        ++relative;

    path += relative;
    if (!path.empty())
        NormalizePathSeparators(&path[0]);

    return path;
}

void CreateDirectories(const char *path)
{
    if (!path || !*path)
        return;

    std::string directory = path;
    NormalizePathSeparators(&directory[0]);

    char *p = &directory[0];
    const bool hasDrivePrefix = directory.size() >= 3 && p[1] == ':' && p[2] == '\\';
    const bool hasGamePrefix = directory.size() >= 6 && strncmp(p, "game:\\", 6) == 0;

    // Skip leading drive letter (e.g., "C:\") or "game:\" prefix
    if (hasDrivePrefix || hasGamePrefix)
        p += (hasDrivePrefix ? 3 : 6); // Move past "C:\" or "game:\"

    for (; *p; p++)
    {
        if (*p == '\\' || *p == '/')
        {
            *p = '\0';
            _mkdir(directory.c_str()); // Attempt to create the directory
            *p = '\\';
        }
    }

    _mkdir(directory.c_str()); // Create final directory
}

void CreateParentDirectories(const char *path)
{
    if (!path || !*path)
        return;

    std::string directory = path;
    NormalizePathSeparators(&directory[0]);

    const size_t lastSeparator = directory.find_last_of('\\');
    if (lastSeparator == std::string::npos)
        return;

    const bool isDriveRoot = lastSeparator == 2 && directory.size() > 1 && directory[1] == ':';
    const bool isGameRoot =
        lastSeparator == 5 && directory.size() >= 6 && strncmp(directory.c_str(), "game:\\", 6) == 0;
    if (isDriveRoot || isGameRoot)
    {
        return;
    }

    directory.erase(lastSeparator);
    CreateDirectories(directory.c_str());
}

/**
 * Writes data to a file on disk.
 *
 * @param filePath The full path to the file to write.
 * @param data The data to write.
 * @param dataSize The size of
 * the data in bytes.
 * @return 1 if successful, 0 if failed.
 */
int WriteFileToDisk(const char *filePath, const char *data, size_t dataSize)
{
    if (!filePath || !data || dataSize == 0)
    {
        DbgPrint("WriteFileToDisk: Invalid parameters!\n");
        return 0;
    }

    CreateParentDirectories(filePath);

    // Write data to file
    FILE *file = fopen(filePath, "wb");
    if (file)
    {
        fwrite(data, 1, dataSize, file);
        fclose(file);
        DbgPrint("Successfully wrote file: %s\n", filePath);
        return 1;
    }
    else
    {
        DbgPrint("Failed to write file: %s\n", filePath);
        return 0;
    }
}

bool FileExists(const char *filePath)
{
    if (!filePath || !*filePath)
        return false;

    const DWORD attributes = GetFileAttributesA(filePath);
    return attributes != static_cast<DWORD>(-1) && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

// Function to read a file's contents into a string
std::string ReadFileToString(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
    {
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

std::vector<std::string> ListFilesInDirectory(const std::string &directory)
{
    std::vector<std::string> filenames;
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA((directory + "\\*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        DbgPrint("ERROR: Directory '%s' does not exist or cannot be accessed.\n", directory.c_str());
        return filenames; // Return empty vector
    }

    do
    {
        // Ignore "." and ".." and only include regular files (not directories)
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            std::string filename(findFileData.cFileName);
            filenames.push_back(filename);
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);

    return filenames;
}
} // namespace filesystem
