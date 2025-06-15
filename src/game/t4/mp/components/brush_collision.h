#pragma once

#include "common.h"

namespace t4
{
    namespace mp
    {
        class BrushCollision : public Module
        {
        public:
            BrushCollision();
            ~BrushCollision();

            const char *get_name() override { return "BrushCollision"; };
        };
    }
}
