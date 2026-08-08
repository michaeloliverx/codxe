#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
void PlayerCmd_ButtonPressed(scr_entref_t entref);
void PlayerCmd_SprintButtonPressed(scr_entref_t entref);
void PlayerCmd_LeanLeftButtonPressed(scr_entref_t entref);
void PlayerCmd_LeanRightButtonPressed(scr_entref_t entref);
void PlayerCmd_JumpButtonPressed(scr_entref_t entref);
void PlayerCmd_HoldBreathButtonPressed(scr_entref_t entref);
void PlayerCmd_NightVisionButtonPressed(scr_entref_t entref);
void PlayerCmd_ForwardButtonPressed(scr_entref_t entref);
void PlayerCmd_BackButtonPressed(scr_entref_t entref);
void PlayerCmd_LeftButtonPressed(scr_entref_t entref);
void PlayerCmd_RightButtonPressed(scr_entref_t entref);
void PlayerCmd_SetVelocity(scr_entref_t entref);
void PlayerCmd_SetStance(scr_entref_t entref);
void GScr_CloneBrushModelToScriptModel(scr_entref_t entref);
void GScr_SetBrushModel(scr_entref_t entref);
} // namespace mp
} // namespace iw3
