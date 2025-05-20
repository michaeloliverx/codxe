#pragma once

namespace common
{
    class Module
    {
    public:
        Module() {};
        virtual ~Module() {};
        virtual const char *get_name() { return "unknown"; }
    };
}
