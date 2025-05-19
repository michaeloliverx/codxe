#include "common.h"

enum GameTitleId
{
	GAME_TITLE_ID_IW3 = 0x415607E6, // Call of Duty 4: Modern Warfare
	GAME_TITLE_ID_T4 = 0x4156081C	// Call of Duty: World at War
};

struct TitleInitEntry
{
	uint32_t title_id;
	void (*init)();
};

static const TitleInitEntry title_init_table[] = {
	{GAME_TITLE_ID_IW3, game::init},
	{GAME_TITLE_ID_T4, t4::init},
};

void monitor_title_id()
{
	for (;;)
	{
		uint32_t current_title_id = xbox::XamGetCurrentTitleId();

		for (size_t i = 0; i < sizeof(title_init_table) / sizeof(title_init_table[0]); ++i)
		{
			const TitleInitEntry &entry = title_init_table[i];
			if (entry.title_id == current_title_id)
			{
				entry.init();
				return;
			}
		}

		Sleep(50);
	}
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
