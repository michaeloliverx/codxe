#pragma once

#include "pch.h"

namespace filesystem
{
std::string JoinPath(const char *basePath, const char *relativePath);
void CreateDirectories(const char *path);
void CreateParentDirectories(const char *path);
int WriteFileToDisk(const char *filePath, const char *data, size_t dataSize);
bool FileExists(const char *filePath);
bool DirectoryExists(const char *directoryPath);
std::string ReadFileToString(const std::string &filePath);
} // namespace filesystem
