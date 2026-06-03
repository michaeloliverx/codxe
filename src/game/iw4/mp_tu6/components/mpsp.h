#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class mpsp : public Module
{
  public:
    mpsp();
    ~mpsp();
    const char *get_name() override
    {
        return "mpsp";
    };
    static bool is_sp_map;
    // static bool is_sp_mapname(const std::string &name);
};
} // namespace mp_tu6
} // namespace iw4
