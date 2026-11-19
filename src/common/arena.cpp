#include "pch.h"
#include "arena.h"

namespace arena
{
std::string ReadText(const std::string &path)
{
    HANDLE file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return std::string();

    DWORD high = 0;
    const DWORD size = GetFileSize(file, &high);
    std::string text;
    if (!high && size > 0 && size <= MAX_FILE_SIZE)
    {
        text.resize(size);
        DWORD read = 0;
        if (!ReadFile(file, &text[0], size, &read, nullptr) || read != size)
            text.clear();
    }
    CloseHandle(file);
    return text;
}

std::string ReadPlainLine(const Fields &fields, const char *key, size_t maxLength)
{
    const Fields::const_iterator field = fields.find(key);
    if (field == fields.end())
        return std::string();

    const std::string &text = field->second;
    const size_t start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return std::string();

    const size_t length = text.find_last_not_of(" \t\r\n") - start + 1;
    if (length > maxLength)
        return std::string();

    // Keep metadata on one line and exclude game control and color codes.
    const std::string label = text.substr(start, length);
    if (label.find('^') != std::string::npos ||
        std::find_if(label.begin(), label.end(), [](unsigned char c) { return c < ' ' || c == 127; }) != label.end())
    {
        return std::string();
    }
    return label;
}
} // namespace arena
