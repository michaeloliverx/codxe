#pragma once

#include "common.h"

namespace iw3
{
    namespace mp
    {
        class cj_tas : public Module
        {
        public:
            cj_tas();
            ~cj_tas();
            const char *get_name() override { return "cj_tas"; };
            static bool TAS_Enabled();
            static void cj_tas::On_CG_Init();
        };
    }
}
