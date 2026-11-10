#include "pch.h"
#include "ui_feeder.h"

namespace iw4
{
namespace mp_tu6
{
std::map<float, UIFeeder::Callbacks> UIFeeder::Feeders;
Detour UIFeeder::UI_FeederCount_Detour;
Detour UIFeeder::UI_FeederItemText_Detour;
Detour UIFeeder::UI_FeederSelection_Detour;

void UIFeeder::Add(float id, const Callbacks &callbacks)
{
    if (callbacks.count && callbacks.text && callbacks.select)
        Feeders[id] = callbacks;
}

int UIFeeder::Count(int localClientNum, float id)
{
    const auto feeder = Feeders.find(id);
    return feeder == Feeders.end() ? UI_FeederCount_Detour.GetOriginal<UI_FeederCount_t>()(localClientNum, id)
                                   : feeder->second.count();
}

const char *UIFeeder::Text(int localClientNum, itemDef_s *item, float id, int index, int column, float *s0, float *t0,
                           float *s1, float *t1, Material **material)
{
    const auto feeder = Feeders.find(id);
    if (feeder == Feeders.end())
        return UI_FeederItemText_Detour.GetOriginal<UI_FeederItemText_t>()(localClientNum, item, id, index, column, s0,
                                                                           t0, s1, t1, material);

    if (material)
        *material = nullptr;
    return index >= 0 && index < feeder->second.count() ? feeder->second.text(localClientNum, index, column) : "";
}

void UIFeeder::Select(int localClientNum, float id, int index)
{
    const auto feeder = Feeders.find(id);
    if (feeder == Feeders.end())
    {
        UI_FeederSelection_Detour.GetOriginal<UI_FeederSelection_t>()(localClientNum, id, index);
        return;
    }
    if (index >= 0 && index < feeder->second.count())
        feeder->second.select(localClientNum, index);
}

UIFeeder::UIFeeder()
{
    UI_FeederCount_Detour = Detour(UI_FeederCount, Count);
    UI_FeederCount_Detour.Install();
    UI_FeederItemText_Detour = Detour(UI_FeederItemText, Text);
    UI_FeederItemText_Detour.Install();
    UI_FeederSelection_Detour = Detour(UI_FeederSelection, Select);
    UI_FeederSelection_Detour.Install();
}

UIFeeder::~UIFeeder()
{
    UI_FeederSelection_Detour.Remove();
    UI_FeederItemText_Detour.Remove();
    UI_FeederCount_Detour.Remove();
    Feeders.clear();
}
} // namespace mp_tu6
} // namespace iw4
