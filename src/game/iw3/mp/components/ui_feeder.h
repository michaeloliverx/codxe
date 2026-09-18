#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class UIFeeder : public Module
{
  public:
    UIFeeder();
    ~UIFeeder();

    static void Add(float feederID, UIFeederGetItemCount_t getItemCount, UIFeederGetItemText_t getItemText,
                    UIFeederSelect_t select);
    static void SetSelectedIndex(float feederID, int index);
    static void OnDvarInit();

  private:
    friend class UIScript;

    static void LoadUsermapsScript(int localClientNum, const char **args);
    static void ApplyMapScript(int localClientNum, const char **args);

    static int UI_FeederCount_Hook(int localClientNum, itemDef_s *item, float feederID);
    static const char *UI_FeederItemText_Hook(int localClientNum, itemDef_s *item, float feederID, int index,
                                              unsigned int column, Material **handle);
    static void UI_FeederItemColor_Hook(int localClientNum, itemDef_s *item, float feederID, int index, int column,
                                        float *color);
    static void UI_FeederSelection_Hook(int localClientNum, float feederID, itemDef_s *item, int index);
    static void Item_ListBox_Scroll_Hook(int localClientNum, itemDef_s *item, int max, int scrollMax, int viewMax,
                                         int delta);

    static std::map<float, UIFeederCallbacks> Feeders;
    static std::map<float, int> SelectedIndices;
    static Detour UI_FeederCount_Detour;
    static Detour UI_FeederItemColor_Detour;
    static Detour UI_FeederItemText_Detour;
    static Detour UI_FeederSelection_Detour;
    static Detour Item_ListBox_Scroll_Detour;
};
} // namespace mp
} // namespace iw3
