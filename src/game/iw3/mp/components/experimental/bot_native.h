#pragma once

namespace iw3
{
namespace mp
{
namespace bot_native
{
void Install();
void Shutdown();
void Reset();
void ClearAim(int clientNum);
void TickAim(int clientNum, client_t *client);
bool TickController(int clientNum, client_t *client, usercmd_s &cmd);
void Control(scr_entref_t entref);
void Pause();
unsigned __int64 BeginBotInput();
void EndBotInput(unsigned __int64 start);
BuiltinFunction ProfileFunction(const char *name, BuiltinFunction original);
BuiltinMethod ProfileMethod(const char *name, BuiltinMethod original);
void NavBegin();
void NavNode();
void NavLink();
void NavEnd();
void NavNearest();
void NavSearch();
void NavUsage();
void AimAt(scr_entref_t entref);
void AimStop(scr_entref_t entref);
void ProfileBegin();
void ProfileEnd();
void ProfileStats();
void ProfileReset();
} // namespace bot_native
} // namespace mp
} // namespace iw3
