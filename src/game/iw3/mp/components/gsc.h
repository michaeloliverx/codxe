#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
void ClearReplacedFunctions();
class GSC : public Module
{
  public:
    GSC();
    ~GSC();

    static void OnVMShutdown();
};
} // namespace mp
} // namespace iw3
