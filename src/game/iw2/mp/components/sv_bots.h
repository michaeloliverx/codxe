#pragma once

#include "pch.h"

namespace iw2
{
namespace mp
{
void GScr_AddTestClient();
void PlayerCmd_BotAction(scr_entref_t entref);
void PlayerCmd_BotStop(scr_entref_t entref);
void PlayerCmd_BotMovement(scr_entref_t entref);
void PlayerCmd_BotAngles(scr_entref_t entref);
void PlayerCmd_IsBot(scr_entref_t entref);
void PlayerCmd_IsHost(scr_entref_t entref);

class SVBots : public Module
{
  public:
    SVBots();
    ~SVBots();

    static void OnVMShutdown();
};
} // namespace mp
} // namespace iw2
