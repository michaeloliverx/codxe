#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class GSCFunctions : public Module
{
  public:
    GSCFunctions();
    ~GSCFunctions();

    static void OnVMShutdown();
};
} // namespace mp
} // namespace t4
