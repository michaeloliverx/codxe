#include <xtl.h>
#include <cstdint>
#include <string>

#include "detour.h"

// Declarations for Xbox 360 kernel functions not exposed in the official SDK.
extern "C"
{
	void DbgPrint(const char *format, ...);

	uint32_t ExCreateThread(
		HANDLE *pHandle,
		uint32_t stackSize,
		uint32_t *pThreadId,
		void *pApiThreadStartup,
		PTHREAD_START_ROUTINE pStartAddress,
		void *pParameter,
		uint32_t creationFlags);

	uint32_t XamGetCurrentTitleId();
}

void *ResolveFunction(const std::string &moduleName, uint32_t ordinal)
{
	HMODULE moduleHandle = GetModuleHandle(moduleName.c_str());
	if (moduleHandle == nullptr)
		return nullptr;

	return GetProcAddress(moduleHandle, reinterpret_cast<const char *>(ordinal));
}

typedef void (*XNOTIFYQUEUEUI)(uint32_t type, uint32_t userIndex, uint64_t areas, const wchar_t *displayText, void *pContextData);
XNOTIFYQUEUEUI XNotifyQueueUI = static_cast<XNOTIFYQUEUEUI>(ResolveFunction("xam.xex", 656));

namespace game
{
	uint32_t XBOX_360_TITLE_ID = 0x415607E6;

	namespace mp
	{
		void init()
		{
			XNotifyQueueUI(0, 0, XNOTIFY_SYSTEM, L"iw3 xenon mp", nullptr);
		}
	}

	namespace sp
	{
		void init()
		{
			XNotifyQueueUI(0, 0, XNOTIFY_SYSTEM, L"iw3 xenon sp", nullptr);
		}
	}

}

void init()
{
	if (strcmp((char *)0x82032AC4, "multiplayer") == 0)
		game::mp::init();
	else
		game::sp::init();
}

void monitor_title_id()
{
	for (;;)
	{
		if (XamGetCurrentTitleId() == game::XBOX_360_TITLE_ID)
		{
			init();
			break;
		}
		else
			Sleep(100);
	}
}

int DllMain(HANDLE hModule, DWORD reason, void *pReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		ExCreateThread(nullptr, 0, nullptr, nullptr, reinterpret_cast<PTHREAD_START_ROUTINE>(monitor_title_id), nullptr, 2);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}
