#include "cg.h"
#include "common.h"

namespace t4
{
    namespace sp
    {

        void DrawBranding(int localClientNum)
        {
            static auto consoleFont = R_RegisterFont("fonts/consoleFont", -1);

            const char *text = branding::GetBrandingString(branding::GAME_T4, branding::MODE_SP);

            const float color_white_rgba[4] = {1.0f, 1.0f, 1.0f, 0.5f}; // RGBA white color

            const float x = -100.f;
            const float y = 10.f;

            UI_DrawText(scrPlaceFullUnsafe, text, 64, consoleFont, x, y, 0, 0, 0.2, color_white_rgba, 0);
        }

        Detour UI_Refresh_Detour;

        void UI_Refresh_Hook(int localClientNum)
        {
            UI_Refresh_Detour.GetOriginal<decltype(UI_Refresh)>()(localClientNum);
            DrawBranding(localClientNum);
        }

        cg::cg()
        {
            UI_Refresh_Detour = Detour(UI_Refresh, UI_Refresh_Hook);
            UI_Refresh_Detour.Install();
        }

        cg::~cg()
        {
            UI_Refresh_Detour.Remove();
        }
    }
}
