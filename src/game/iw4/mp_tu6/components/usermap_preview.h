#pragma once
#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class UsermapPreview : public Module
{
  public:
    UsermapPreview();
    ~UsermapPreview();
    static bool Load(const std::string &path);
};
} // namespace mp_tu6
} // namespace iw4
