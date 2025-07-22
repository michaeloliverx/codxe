#include "common.h"

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)
#endif

namespace filesystem
{
    const char WINDOWS_PATH_SEPARATOR = '\\';
    const char UNIX_PATH_SEPARATOR = '/';

    void create_nested_dirs(const char *path)
    {
        if (!path || !*path)
            return;

        char temp_path[256];
        strncpy(temp_path, path, sizeof(temp_path) - 1);
        temp_path[sizeof(temp_path) - 1] = '\0';

        char *p = temp_path;

        // Skip leading drive letter (e.g., "C:\") or "game:\" prefix
        if ((p[0] && p[1] == ':') || strncmp(p, "game:\\", 6) == 0)
            p += (p[1] == ':' ? 3 : 6); // Move past "C:\" or "game:\"

        for (; *p; p++)
        {
            if (*p == '\\' || *p == '/')
            {
                *p = '\0';
                _mkdir(temp_path); // Attempt to create the directory
                *p = '\\';
            }
        }

        _mkdir(temp_path); // Create final directory
    }

    /**
     * Writes data to a file on disk.
     *
     * @param file_path The full path to the file to write.
     * @param data The data to write.
     * @param data_size The size of the data in bytes.
     * @return 1 if successful, 0 if failed.
     */
    int write_file_to_disk(const char *file_path, const char *data, size_t data_size)
    {
        if (!file_path || !data || data_size == 0)
        {
            DbgPrint("write_file_to_disk: Invalid parameters!\n");
            return 0;
        }

        // Ensure the directory exists
        char dir_path[256];
        strncpy(dir_path, file_path, sizeof(dir_path));
        char *last_slash = strrchr(dir_path, '\\');
        if (last_slash)
        {
            *last_slash = '\0';
            create_nested_dirs(dir_path);
        }

        // Write data to file
        FILE *file = fopen(file_path, "wb");
        if (file)
        {
            fwrite(data, 1, data_size, file);
            fclose(file);
            DbgPrint("Successfully wrote file: %s\n", file_path);
            return 1;
        }
        else
        {
            DbgPrint("Failed to write file: %s\n", file_path);
            return 0;
        }
    }

    // Function to check if a file exists
    bool file_exists(const std::string &file_path)
    {
        std::ifstream file(file_path.c_str());
        return file.good();
    }

    // Function to read a file's contents into a string
    std::string read_file_to_string(const std::string &file_path)
    {
        std::ifstream file(file_path, std::ios::binary);
        if (!file)
        {
            DbgPrint("read_file_to_string: Failed to open file: %s\n", file_path.c_str());
            return "";
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        return content;
    }

    std::vector<std::string> list_files_in_directory(const std::string &directory)
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

    bool exists(const std::string &p)
    {
        return GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES;
    }

    bool create_directory(const std::string &p)
    {
        return CreateDirectoryA(p.c_str(), NULL) != 0;
    }

    bool create_directories(const std::string &path)
    {
        if (path.empty())
            return false;

        char temp[MAX_PATH];
        strncpy(temp, path.c_str(), MAX_PATH);
        temp[MAX_PATH - 1] = '\0'; // ensure null termination

        // Normalize slashes
        for (char *p = temp; *p; ++p)
        {
            if (*p == UNIX_PATH_SEPARATOR)
                *p = WINDOWS_PATH_SEPARATOR;
        }

        char *start = temp;

        // Skip drive segments by looking for first backslash after colon
        char *colon = strchr(temp, ':');
        if (colon)
        {
            char *firstSlash = strchr(colon, WINDOWS_PATH_SEPARATOR);
            if (firstSlash)
            {
                start = firstSlash + 1; // Skip past the first backslash after colon
            }
        }

        // Step through the path, creating each part
        for (char *p = start; *p; ++p)
        {
            if (*p == WINDOWS_PATH_SEPARATOR)
            {
                *p = '\0';
                CreateDirectoryA(temp, NULL); // ignore failure if already exists
                *p = WINDOWS_PATH_SEPARATOR;
            }
        }

        // Create the full directory
        return CreateDirectoryA(temp, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
    }

    bool is_directory(const std::string &p)
    {
        DWORD attributes = GetFileAttributesA(p.c_str());
        return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY));
    }

    bool is_regular_file(const std::string &p)
    {
        DWORD attributes = GetFileAttributesA(p.c_str());
        return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
    }
}
