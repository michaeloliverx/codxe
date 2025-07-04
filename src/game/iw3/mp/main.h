#pragma once

#include "common.h"

namespace iw3
{
    class Module
    {
    public:
        Module() {};
        virtual ~Module() {};
        virtual const char *get_name() { return "unknown"; }
    };
    namespace mp
    {
        void init();
        void shutdown();
    }
}
