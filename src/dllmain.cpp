#include <xtl.h>
#include <cstdint>
#include <string>
#include "xboxkrnl.h"
#include "game/game.h"

void monitor_title_id()
{
	while (xbox::XamGetCurrentTitleId() != game::XBOX_360_TITLE_ID)
		Sleep(100);

	// If not running in an emulator, sleep for 500ms before initialization
	// to allow the game to load properly.
	if (!xbox::InXenia())
	{
		Sleep(500);
	}

	game::init();
}

int DllMain(HANDLE hModule, DWORD reason, void *pReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		xbox::ExCreateThread(nullptr, 0, nullptr, nullptr, reinterpret_cast<PTHREAD_START_ROUTINE>(monitor_title_id), nullptr, 2);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}
