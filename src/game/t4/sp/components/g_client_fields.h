#pragma once

#include "common.h"

namespace t4
{
    namespace sp
    {
        class g_client_fields : public Module
        {
        public:
            g_client_fields();
            ~g_client_fields();

            const char *get_name() override { return "g_client_fields"; };
        };
    }
}
