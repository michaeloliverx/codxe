#include "pch.h"
#include "usermaps.h"

namespace usermaps
{
namespace
{
bool EndsWith(const std::string &text, const char *suffix)
{
    const size_t length = std::strlen(suffix);
    return text.size() >= length && _stricmp(text.c_str() + text.size() - length, suffix) == 0;
}
} // namespace

const char *GetDirectory()
{
    return "codxe:\\usermaps";
}

bool IsSafeName(const char *name)
{
    if (!name || !*name || std::strcmp(name, ".") == 0 || std::strcmp(name, "..") == 0)
        return false;
    for (const char *c = name; *c; ++c)
    {
        const bool letter = (*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z');
        if (!letter && !(*c >= '0' && *c <= '9') && *c != '_' && *c != '-' && *c != '.')
            return false;
    }
    return std::strlen(name) < 64;
}

bool IsMultiplayerName(const char *name)
{
    return IsSafeName(name) && _strnicmp(name, "mp_", 3) == 0 && name[3] != '\0';
}

std::string FastfilePath(const char *zoneName)
{
    if (!zoneName || std::strlen(zoneName) >= 80)
        return std::string();
    std::string filename = zoneName;
    if (EndsWith(filename, ".ff"))
        filename.erase(filename.size() - 3);
    std::string folder = filename;
    if (EndsWith(folder, "_load"))
        folder.erase(folder.size() - 5);
    else if (EndsWith(folder, "_patch"))
        folder.erase(folder.size() - 6);
    if (!IsMultiplayerName(folder.c_str()))
        return std::string();
    const std::string directory = filesystem::JoinPath(GetDirectory(), folder.c_str());
    return filesystem::JoinPath(directory.c_str(), (filename + ".ff").c_str());
}

bool ReadMetadata(const std::string &directory, const std::string &map, std::string &name, std::string &description)
{
    arena::Fields fields;
    if (!ReadMetadata(directory, map, fields))
        return false;
    name = fields["longname"];
    description = fields["description"];
    return true;
}

bool ReadMetadata(const std::string &directory, const std::string &map, arena::Fields &fields)
{
    const char *filenames[] = {"map.arena", nullptr};
    const std::string named = map + ".arena";
    filenames[1] = named.c_str();
    for (size_t i = 0; i < ARRAYSIZE(filenames); ++i)
    {
        const std::string text = arena::ReadText(filesystem::JoinPath(directory.c_str(), filenames[i]));
        if (arena::Parse(text, map, fields))
            return true;
    }
    return false;
}

Credits ReadCredits(const arena::Fields &fields)
{
    Credits credits;
    credits.author = arena::ReadPlainLine(fields, "author", 40);
    credits.revision = arena::ReadPlainLine(fields, "revision", 24);
    return credits;
}

std::vector<Entry> ScanMultiplayer()
{
    std::vector<Entry> result;
    WIN32_FIND_DATAA data;
    const std::string pattern = filesystem::JoinPath(GetDirectory(), "*");
    HANDLE search = FindFirstFileA(pattern.c_str(), &data);
    if (search == INVALID_HANDLE_VALUE)
        return result;
    do
    {
        if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !IsMultiplayerName(data.cFileName))
            continue;
        const std::string map = data.cFileName;
        // Companion-zone folders are never standalone map entries.
        if (EndsWith(map, "_load") || EndsWith(map, "_patch"))
            continue;
        const std::string directory = filesystem::JoinPath(GetDirectory(), map.c_str());
        const std::string main = filesystem::JoinPath(directory.c_str(), (map + ".ff").c_str());
        if (!filesystem::FileExists(main.c_str()))
            continue;
        Entry entry;
        entry.name = map;
        arena::Fields fields;
        ReadMetadata(directory, map, fields);
        entry.displayName = fields["longname"];
        entry.description = fields["description"];
        entry.credits = ReadCredits(fields);
        if (entry.displayName.empty())
            entry.displayName = map;
        const std::string preview = filesystem::JoinPath(directory.c_str(), "preview.dds");
        if (filesystem::FileExists(preview.c_str()))
            entry.previewPath = preview;
        result.push_back(entry);
    } while (FindNextFileA(search, &data));
    FindClose(search);
    std::sort(result.begin(), result.end(), [](const Entry &a, const Entry &b)
              { return _stricmp(a.displayName.c_str(), b.displayName.c_str()) < 0; });
    return result;
}
} // namespace usermaps
