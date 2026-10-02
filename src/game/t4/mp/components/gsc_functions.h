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
    static void ClearReplacedFunctions();
};
} // namespace mp
} // namespace t4
