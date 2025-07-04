#pragma once

#include "common.h"

namespace iw3
{
    namespace mp
    {
        class pm : public Module
        {
        public:
            pm();
            ~pm();

            const char *get_name() override { return "pm"; };
        };
    }
}
