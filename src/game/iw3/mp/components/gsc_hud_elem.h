#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
void Scr_PrecacheString_Stub();
void HECmd_SetText_Stub(scr_entref_t entref);

class gsc_hud_elem : public Module
{
  public:
    gsc_hud_elem();
    ~gsc_hud_elem();

    const char *get_name() override
    {
        return "gsc_hud_elem";
    };
};
} // namespace mp
} // namespace iw3
