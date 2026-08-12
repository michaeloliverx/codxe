#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class mpsp : public Module
{
  public:
    mpsp();
    ~mpsp();
    static bool is_sp_map;
    static bool is_sp_mapname(const std::string &name);
    static void OnAssetLink(XAsset *asset);
};
} // namespace mp
} // namespace iw3
