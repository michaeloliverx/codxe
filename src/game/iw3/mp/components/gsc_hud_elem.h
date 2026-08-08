#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
void Scr_PrecacheString_Stub();
void HECmd_SetText_Stub(scr_entref_t entref);

class GSCHudElem : public Module
{
  public:
    GSCHudElem();
    ~GSCHudElem();

    const char *get_name() override
    {
        return "GSCHudElem";
    };
};
} // namespace mp
} // namespace iw3
