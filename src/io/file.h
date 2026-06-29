#pragma once

#include <cstddef>

namespace codxe
{

bool FileExists(const char *path);
bool CreateDirectories(const char *path);
bool CreateDirectoriesForFile(const char *path);
bool ReadFileToBuffer(const char *path, void *buffer, size_t buffer_size, size_t *bytes_read);
bool WriteFileToDisk(const char *path, const void *data, size_t data_size);

} // namespace codxe
