#pragma once

#include "../modules.h"

namespace t4
{
    namespace mp
    {
        class GSCLoader : public Module
        {
        public:
            GSCLoader();
            ~GSCLoader();

            const char *get_name() override { return "GSCLoader"; };
        };
    }
}
