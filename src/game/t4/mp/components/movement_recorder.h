#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
class MovementRecorder : public Module
{
  public:
    MovementRecorder();
    ~MovementRecorder();

    static void OnCmdInit();
    static void OnCGInit(int localClientNum);
};
} // namespace mp
} // namespace t4
