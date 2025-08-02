#pragma once

#include "common.h"

namespace iw3
{
    namespace sp
    {
        class scr_parser : public Module
        {
        public:
            scr_parser();
            ~scr_parser();

            const char *get_name() override { return "scr_parser"; };
        };
    }
}
