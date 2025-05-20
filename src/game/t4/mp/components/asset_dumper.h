#pragma once

#include "common/module.h"
#include <string>

namespace t4
{
    namespace mp
    {
        class AssetDumperModule : public common::Module
        {
        public:
            AssetDumperModule();
            ~AssetDumperModule();

            const char *get_name() override { return "AssetDumper"; };
        };
    }
}
