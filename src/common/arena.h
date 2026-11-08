#pragma once

#include <string>
#include <cctype>

namespace arena
{
class Tokens
{
    const std::string &text;
    size_t offset;
    Tokens &operator=(const Tokens &);

  public:
    bool valid;
    bool quoted;
    explicit Tokens(const std::string &value) : text(value), offset(0), valid(true), quoted(false)
    {
        if (text.compare(0, 3, "\xEF\xBB\xBF") == 0)
            offset = 3;
    }

    bool Next(std::string &token)
    {
        token.clear();
        quoted = false;
        while (offset < text.size())
        {
            if (std::isspace(static_cast<unsigned char>(text[offset])))
                ++offset;
            else if (text.compare(offset, 2, "//") == 0)
            {
                offset = text.find('\n', offset);
                if (offset == std::string::npos)
                    offset = text.size();
            }
            else if (text.compare(offset, 2, "/*") == 0)
            {
                const size_t end = text.find("*/", offset + 2);
                if (end == std::string::npos)
                {
                    valid = false;
                    return false;
                }
                offset = end + 2;
            }
            else
                break;
        }
        if (offset == text.size())
            return false;
        const char first = text[offset++];
        if (first == '{' || first == '}')
        {
            token += first;
            return true;
        }
        if (first == '"')
        {
            quoted = true;
            while (offset < text.size())
            {
                char c = text[offset++];
                if (c == '"')
                    return true;
                if (c == '\\' && offset < text.size())
                {
                    const char escaped = text[offset];
                    if (escaped == '"' || escaped == '\\')
                    {
                        c = escaped;
                        ++offset;
                    }
                    else if (escaped == 'n' || escaped == 'r' || escaped == 't')
                    {
                        c = escaped == 'n' ? '\n' : (escaped == 'r' ? '\r' : '\t');
                        ++offset;
                    }
                }
                token += c;
            }
            valid = false;
            return false;
        }
        token += first;
        while (offset < text.size() && !std::isspace(static_cast<unsigned char>(text[offset])) && text[offset] != '{' &&
               text[offset] != '}')
            token += text[offset++];
        return true;
    }
};

inline std::string Lower(std::string value)
{
    for (size_t i = 0; i < value.size(); ++i)
        value[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(value[i])));
    return value;
}

// Metadata never changes the folder/fastfile used to launch a map.
inline bool Parse(const std::string &text, const std::string &folder, std::string &name, std::string &description)
{
    if (text.size() > 65536 || text.find('\0') != std::string::npos)
        return false;
    Tokens tokens(text);
    std::string token, selectedName, selectedDescription;
    bool found = false;
    while (tokens.Next(token))
    {
        if (token != "{" || tokens.quoted)
            return false;
        std::string map, longname, detail;
        bool closed = false;
        while (tokens.Next(token))
        {
            if (token == "}" && !tokens.quoted)
            {
                closed = true;
                break;
            }
            if ((token == "{" && !tokens.quoted) || token.empty())
                return false;
            const std::string key = Lower(token);
            if (!tokens.Next(token) || (!tokens.quoted && (token == "{" || token == "}")))
                return false;
            if (key == "map")
                map = token;
            else if (key == "longname")
                longname = token;
            else if (key == "description")
                detail = token;
        }
        if (!closed)
            return false;
        if (!found && Lower(map) == Lower(folder))
        {
            found = true;
            selectedName = longname;
            selectedDescription = detail;
        }
    }
    if (!tokens.valid || !found)
        return false;
    name = selectedName;
    description = selectedDescription;
    return true;
}
} // namespace arena
