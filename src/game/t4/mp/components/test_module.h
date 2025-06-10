#pragma once

#include "../modules.h"

namespace t4
{
    namespace mp
    {
        class TestModule : public Module
        {
        public:
            TestModule();
            ~TestModule();

            const char *get_name() override { return "TestModule"; };
        };
    }
}
