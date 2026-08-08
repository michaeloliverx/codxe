#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
void GScr_AddTestClient();
void Scr_BotMoveTo(scr_entref_t entref);
void Scr_BotAction(scr_entref_t entref);
void Scr_BotMirror(scr_entref_t entref);
void Scr_BotStop(scr_entref_t entref);

class sv_bots : public Module
{
  public:
    sv_bots();
    ~sv_bots();

    const char *get_name() override
    {
        return "sv_bots";
    };
};
} // namespace mp
} // namespace iw3
