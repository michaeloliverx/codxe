#pragma once

#include "common.h"

namespace t4
{
    namespace mp
    {
        class GSCClientMethods : public Module
        {
        public:
            GSCClientMethods();
            ~GSCClientMethods();

            const char *get_name() override { return "GSCClientMethods"; };
        };
    }
}
