#pragma once

#include <string>

namespace iw3
{
namespace mp
{
namespace streamed_paths
{
inline bool EqualAscii(char a, char b)
{
    if (a >= 'A' && a <= 'Z')
        a += 'a' - 'A';
    if (b >= 'A' && b <= 'Z')
        b += 'a' - 'A';
    return a == b;
}

inline bool StartsWith(const std::string &value, const char *prefix)
{
    for (size_t i = 0; prefix[i]; ++i)
        if (i >= value.size() || !EqualAscii(value[i], prefix[i]))
            return false;
    return true;
}

inline bool SafeRelativePath(const std::string &path)
{
    if (path.empty())
        return false;
    size_t start = 0;
    for (size_t i = 0; i <= path.size(); ++i)
    {
        if (i == path.size() || path[i] == '\\')
        {
            const std::string part = path.substr(start, i - start);
            if (part.empty() || part == "." || part == ".." || part[part.size() - 1] == '.' ||
                part[part.size() - 1] == ' ')
                return false;
            start = i + 1;
        }
        else if (static_cast<unsigned char>(path[i]) < ' ' || path[i] == ':' || path[i] == '/' ||
                 path[i] == '*' || path[i] == '?' || path[i] == '"' || path[i] == '<' || path[i] == '>' ||
                 path[i] == '|')
            return false;
    }
    return true;
}

inline std::string RelativePath(const char *filename, bool highmip)
{
    if (!filename || !*filename)
        return std::string();
    std::string request = filename;
    for (size_t i = 0; i < request.size(); ++i)
        if (request[i] == '/')
            request[i] = '\\';

    // TU4 builds disk requests from cwd (D:), not from the menu's selected map.
    if (StartsWith(request, "D:\\"))
        request.erase(0, 3);
    else if (!highmip)
        return std::string();

    const char *prefix = highmip ? "highmip\\" : "sounds\\";
    const char *extension = highmip ? ".hi" : ".xma";
    const size_t extensionLength = highmip ? 3 : 4;
    const size_t basename = request.find_last_of('\\');
    if (!StartsWith(request, prefix) || !SafeRelativePath(request) || basename == std::string::npos ||
        request.size() - basename - 1 <= extensionLength ||
        !StartsWith(request.substr(request.size() - extensionLength), extension))
        return std::string();
    if (highmip && request.find('\\', 8) != std::string::npos)
        return std::string();
    return request;
}

inline std::string Resolve(const std::string &relative, const std::string &mapDirectory,
                           const std::string &modDirectory, bool (*exists)(const char *))
{
    if (!SafeRelativePath(relative))
        return std::string();
    if (!mapDirectory.empty())
    {
        const std::string path = mapDirectory + "\\" + relative;
        if (exists(path.c_str()))
            return path;
    }
    if (!modDirectory.empty())
    {
        const std::string path = modDirectory + "\\" + relative;
        if (exists(path.c_str()))
            return path;
    }
    return std::string();
}
} // namespace streamed_paths
} // namespace mp
} // namespace iw3
