#pragma once

#include "common.h"

namespace iw4
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
