#pragma once

#include "pch.h"

namespace t4
{
namespace mp
{
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

void PlayerCmd_BotAction(scr_entref_t entref);
void PlayerCmd_BotStop(scr_entref_t entref);
void PlayerCmd_BotMovement(scr_entref_t entref);
void PlayerCmd_BotMeleeParams(scr_entref_t entref);
void PlayerCmd_BotAngles(scr_entref_t entref);
void PlayerCmd_IsBot(scr_entref_t entref);
void PlayerCmd_IsHost(scr_entref_t entref);

} // namespace mp
} // namespace t4
