#pragma once

#include "common.h"

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

        private:
            static char *Scr_AddSourceBuffer_Hook(scriptInstance_t a1, const char *filename, const char *extFilename, const char *codePos, bool archive);
        };
    }
}
