#pragma once

#include "../modules.h"

namespace t4
{
    namespace mp
    {
        class Branding : public Module
        {
        public:
            Branding();
            ~Branding();

            const char *get_name() override { return "Branding"; };

        private:
            static void DrawBranding();
            static void UI_DrawBuildNumber_Hook(const int localClientNum);
        };
    }
}
