#include "pch.h"
#include "string_utils.h"

namespace string_utils
{
bool EndsWith(const std::string &value, const char *suffix)
{
    if (!suffix)
        return false;

    const size_t suffixLength = std::strlen(suffix);
    return value.length() >= suffixLength && _stricmp(value.c_str() + value.length() - suffixLength, suffix) == 0;
}
} // namespace string_utils
