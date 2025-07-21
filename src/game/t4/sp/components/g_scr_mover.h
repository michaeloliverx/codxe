#pragma once

#include "common.h"

namespace t4
{
    namespace sp
    {
        class g_scr_mover : public Module
        {
        public:
            g_scr_mover();
            ~g_scr_mover();

            const char *get_name() override { return "g_scr_mover"; };
        };
    }
}
