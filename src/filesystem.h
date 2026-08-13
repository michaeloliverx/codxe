#pragma once

#include "pch.h"

namespace filesystem
{
void NormalizePathSeparators(char *path);
std::string JoinPath(const char *basePath, const char *relativePath);
void CreateDirectories(const char *path);
void CreateParentDirectories(const char *path);
int WriteFileToDisk(const char *filePath, const char *data, size_t dataSize);
bool FileExists(const char *filePath);
std::string ReadFileToString(const std::string &filePath);
std::vector<std::string> ListFilesInDirectory(const std::string &directory);
} // namespace filesystem
