#pragma once

#include "pch.h"
namespace iw5
{
namespace mp
{

void GScr_AddTestClient();
void ResetBotState();
void PlayerCmd_BotAction(scr_entref_t entref);
void PlayerCmd_BotStop(scr_entref_t entref);
void PlayerCmd_BotMovement(scr_entref_t entref);
void PlayerCmd_BotMeleeParams(scr_entref_t entref);
void PlayerCmd_BotRemoteAngles(scr_entref_t entref);
void PlayerCmd_BotAngles(scr_entref_t entref);
class Bots : public Module
{
  public:
    Bots();
    ~Bots();
};
} // namespace mp
} // namespace iw5
