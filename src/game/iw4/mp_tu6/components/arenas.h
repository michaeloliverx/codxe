#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class Arenas : public Module
{
  public:
    Arenas();
    ~Arenas();

    // Rescanning is explicit; native lookups also lazily support directly loaded maps.
    static void Reload();
    static bool GetField(const char *map, const char *field, std::string &value);
    static std::string DisplayName(const char *map);
    static std::string Description(const char *map);
};
} // namespace mp_tu6
} // namespace iw4
