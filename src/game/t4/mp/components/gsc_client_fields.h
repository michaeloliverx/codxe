#pragma once

#include "common.h"

namespace t4
{
    namespace mp
    {
        class GSCClientFields : public Module
        {
        public:
            GSCClientFields();
            ~GSCClientFields();

            const char *get_name() override { return "GSCClientFields"; };
        };
    }
}
