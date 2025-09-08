#pragma once

#include "common.h"

namespace iw4
{
namespace mp
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
} // namespace mp
} // namespace iw4
