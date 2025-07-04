#pragma once

#include "common.h"

namespace iw3
{
    namespace mp
    {
        class clipmap : public Module
        {
        public:
            clipmap();
            ~clipmap();

            const char *get_name() override { return "clipmap"; };
            static void HandleBrushCollisionChange();
        };
    }
}
