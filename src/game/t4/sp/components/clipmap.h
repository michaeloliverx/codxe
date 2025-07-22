#pragma once

#include "common.h"

namespace t4
{
    namespace sp
    {
        class clipmap : public Module
        {
        public:
            clipmap();
            ~clipmap();

            const char *get_name() override { return "clipmap"; };
        };
    }
}
