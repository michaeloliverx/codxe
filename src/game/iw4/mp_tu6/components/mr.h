#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class MovementRecorder : public Module
{
  public:
    MovementRecorder();
    ~MovementRecorder();
    const char *get_name() override
    {
        return "MovementRecorder";
    };
    static bool TAS_Enabled();
    static void MovementRecorder::On_CG_Init();
};
} // namespace mp_tu6
} // namespace iw4
