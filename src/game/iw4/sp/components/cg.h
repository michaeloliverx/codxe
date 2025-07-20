#pragma once

#include "common.h"

namespace iw4
{
    namespace sp
    {
        class cg : public Module
        {
        public:
            cg();
            ~cg();

            const char *get_name() override { return "cg"; };
        };
    }
}
