#pragma once

#include "pch.h"

class clipmap : public Module
{
  public:
    clipmap();
    ~clipmap();

    static void OnCGDrawActive(int localClientNum);
};

void DisablePlayerClipOnIntersectingBrushes(iw4::mp_tu6::scr_entref_t entref);
