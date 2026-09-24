#include "pch.h"
#include "ui.h"
#include "console.h"

#include <cstdlib>
#include <cstdio>

namespace t4
{
namespace sp
{
void DrawBranding(int localClientNum)
{
    static auto consoleFont = R_RegisterFont("fonts/consoleFont", -1);

    const char *text = branding::GetBrandingString();

    const float color_white_rgba[4] = {1.0f, 1.0f, 1.0f, 0.5f}; // RGBA white color

    const float x = -100.f;
    const float y = 10.f;

    UI_DrawText(scrPlaceFullUnsafe, text, 64, consoleFont, x, y, 0, 0, 0.2, color_white_rgba, 0);
}

Detour UI_Refresh_Detour;

void UI_Refresh_Hook(int localClientNum)
{
    UI_Refresh_Detour.GetOriginal<decltype(UI_Refresh)>()(localClientNum);
    console::OnUIRefresh();
    DrawBranding(localClientNum);
}

void Campaign_UnlockAll()
{
    Cbuf_AddText(0, "set mis_01 50\n");
}

Detour Menus_OpenByName_Detour;
Detour Item_Slider_HandleKey_Detour;

// Stock sliders move by 5% of their range; FOV needs one-degree steps.
int Item_Slider_HandleKey_Hook(UiContext *dc, itemDef_s *item, int key)
{
    const auto original = Item_Slider_HandleKey_Detour.GetOriginal<decltype(Item_Slider_HandleKey)>();

    int direction = 0;
    if (key == K_DPAD_LEFT || key == K_APAD_LEFT || key == K_LEFTARROW || key == K_PGUP)
        direction = -1;
    else if (key == K_DPAD_RIGHT || key == K_APAD_RIGHT || key == K_RIGHTARROW || key == K_PGDN)
        direction = 1;

    if (!item || !direction)
        return original(dc, item, key);

    const auto *dvarName = item->dvar;
    if (!dvarName || std::strcmp(dvarName, "cg_fov") != 0)
        return original(dc, item, key);

    const auto *limits = item->typeData.editField;
    if (!limits)
        return original(dc, item, key);

    const float before = static_cast<float>(std::atof(Dvar_GetVariantString(dvarName)));
    const int handled = original(dc, item, key);
    if (handled)
    {
        float after = before + static_cast<float>(direction);
        if (after < limits->minVal)
            after = limits->minVal;
        else if (after > limits->maxVal)
            after = limits->maxVal;

        char value[32];
        sprintf_s(value, "%g", after);
        Dvar_SetFromStringByName(dvarName, value);
    }

    return handled;
}

void Menus_OpenByName_Hook(UiContext *dc, const char *menuName)
{
    if (std::strcmp(menuName, "main_solo") == 0      // solo mission select
        || std::strcmp(menuName, "main_online") == 0 // coop mission select
    )
        Campaign_UnlockAll();

    Menus_OpenByName_Detour.GetOriginal<decltype(Menus_OpenByName)>()(dc, menuName);
}

ui::ui()
{
    UI_Refresh_Detour = Detour(UI_Refresh, UI_Refresh_Hook);
    UI_Refresh_Detour.Install();

    Menus_OpenByName_Detour = Detour(Menus_OpenByName, Menus_OpenByName_Hook);
    Menus_OpenByName_Detour.Install();

    Item_Slider_HandleKey_Detour = Detour(Item_Slider_HandleKey, Item_Slider_HandleKey_Hook);
    Item_Slider_HandleKey_Detour.Install();
}

ui::~ui()
{
    Item_Slider_HandleKey_Detour.Remove();
    Menus_OpenByName_Detour.Remove();
    UI_Refresh_Detour.Remove();
}
} // namespace sp
} // namespace t4
