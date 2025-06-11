#pragma once

#include "common.h"

namespace t4
{
    namespace mp
    {
        class GSCFunctions : public Module
        {
        public:
            GSCFunctions();
            ~GSCFunctions();
            const char *get_name() override { return "GSCFunctions"; };
        };
    }
}
