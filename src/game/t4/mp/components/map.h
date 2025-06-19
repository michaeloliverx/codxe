#pragma once

#include "common.h"

namespace t4
{
    namespace mp
    {
        class Map : public Module
        {
        public:
            Map();
            ~Map();

            const char *get_name() override { return "Map"; };
        };
    }
}
