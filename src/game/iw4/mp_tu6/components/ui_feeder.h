#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
class UIFeeder : public Module
{
  public:
    struct Callbacks
    {
        int (*count)();
        const char *(*text)(int localClientNum, int index, int column);
        void (*select)(int localClientNum, int index);
    };

    UIFeeder();
    ~UIFeeder();
    static void Add(float id, const Callbacks &callbacks);

  private:
    static int Count(int localClientNum, float id);
    static const char *Text(int localClientNum, itemDef_s *item, float id, int index, int column, float *s0, float *t0,
                            float *s1, float *t1, Material **material);
    static void Select(int localClientNum, float id, int index);
    static std::map<float, Callbacks> Feeders;
    static Detour UI_FeederCount_Detour;
    static Detour UI_FeederItemText_Detour;
    static Detour UI_FeederSelection_Detour;
};
} // namespace mp_tu6
} // namespace iw4
