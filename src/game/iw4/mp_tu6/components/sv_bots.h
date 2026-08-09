#pragma once

#include "pch.h"

namespace iw4
{
namespace mp_tu6
{
void GScr_AddTestClient();
void PlayerCmd_BotAction(scr_entref_t entref);
void PlayerCmd_BotStop(scr_entref_t entref);
void PlayerCmd_BotMovement(scr_entref_t entref);
void PlayerCmd_BotMeleeParams(scr_entref_t entref);
void PlayerCmd_BotRemoteAngles(scr_entref_t entref);
void PlayerCmd_BotAngles(scr_entref_t entref);

class SVBots : public Module
{
  public:
    SVBots();
    ~SVBots();

    const char *get_name() override
    {
        return "SVBots";
    };
};
} // namespace mp_tu6
} // namespace iw4
