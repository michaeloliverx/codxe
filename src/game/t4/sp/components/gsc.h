#pragma once

#include "common.h"

namespace t4
{
    namespace sp
    {
        class GSC : public Module
        {
        public:
            GSC();
            ~GSC();

            const char *get_name() override { return "GSC"; };
        };
    }
}
