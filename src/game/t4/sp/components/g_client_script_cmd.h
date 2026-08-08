#pragma once

#include "pch.h"

namespace t4
{
namespace sp
{
void PlayerCmd_JumpButtonPressed(scr_entref_t entref);
void PlayerCmd_secondaryOffhandButtonPressed(scr_entref_t entref);
void PlayerCmd_SprintButtonPressed(scr_entref_t entref);
void PlayerCmd_MoveForwardButtonPressed(scr_entref_t entref);
void PlayerCmd_MoveBackButtonPressed(scr_entref_t entref);
void PlayerCmd_MoveLeftButtonPressed(scr_entref_t entref);
void PlayerCmd_MoveRightButtonPressed(scr_entref_t entref);

class g_client_script_cmd : public Module
{
  public:
    g_client_script_cmd();
    ~g_client_script_cmd();
    const char *get_name() override
    {
        return "g_client_script_cmd";
    };
};
} // namespace sp
} // namespace t4
