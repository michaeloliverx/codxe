#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class GSC : public Module
{
  public:
    GSC();
    ~GSC();

    static void OnVMShutdown();
};
} // namespace mp_tu6
} // namespace iw4
