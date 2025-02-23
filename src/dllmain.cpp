#include <xtl.h>
#include <cstdint>
#include <string>
#include <ppcintrinsics.h>

#include "mp\structs.h"
#include "detour.h"
#include "xboxkrnl.h"

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
		// Functions
		CL_ConsoleCharEvent_t CL_ConsoleCharEvent = reinterpret_cast<CL_ConsoleCharEvent_t>(0x822D7368);
		CL_GamepadButtonEvent_t CL_GamepadButtonEvent = reinterpret_cast<CL_GamepadButtonEvent_t>(0x822DD1E8);
		CL_GamepadButtonEventForPort_t CL_GamepadButtonEventForPort = reinterpret_cast<CL_GamepadButtonEventForPort_t>(0x822DD910);
		Com_GetLocalClientNumForEventOnPort_t Com_GetLocalClientNumForEventOnPort = reinterpret_cast<Com_GetLocalClientNumForEventOnPort_t>(0x82235BD8);
		IN_GamepadsMove_t IN_GamepadsMove = reinterpret_cast<IN_GamepadsMove_t>(0x82352A50);
		Key_SetCatcher_t Key_SetCatcher = reinterpret_cast<Key_SetCatcher_t>(0x822D6420);
		Sys_Milliseconds_t Sys_Milliseconds = reinterpret_cast<Sys_Milliseconds_t>(0x821A3C70);

		// Variables
		auto con = reinterpret_cast<game::mp::Console *>(0x82407590);
		auto cl_controller_in_use = reinterpret_cast<int *>(0x850274B8);
		auto cl_last_controller_input = reinterpret_cast<int *>(0x850274BC);
		auto cl_multi_gamepads_enabled = reinterpret_cast<int *>(0x823A46E4);
		auto s_gamePads = reinterpret_cast<GamePad *>(0x84C00DB8);
		auto clientUIActives = reinterpret_cast<clientUIActive_t *>(0x82435A18);

		const unsigned __int8 virtualKeyConvert[146][2] =
			{
				{0u, 0u},
				{200u, 200u},
				{201u, 201u},
				{0u, 0u},
				{202u, 202u},
				{203u, 203u},
				{204u, 204u},
				{0u, 0u},
				{127u, 127u},
				{9u, 9u},
				{0u, 0u},
				{0u, 0u},
				{186u, 0u},
				{13u, 191u},
				{0u, 0u},
				{0u, 0u},
				{160u, 160u},
				{159u, 159u},
				{158u, 158u},
				{153u, 153u},
				{151u, 151u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{27u, 27u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{32u, 32u},
				{184u, 164u},
				{190u, 163u},
				{188u, 166u},
				{182u, 165u},
				{185u, 156u},
				{183u, 154u},
				{187u, 157u},
				{189u, 155u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{192u, 161u},
				{193u, 162u},
				{0u, 0u},
				{48u, 48u},
				{49u, 49u},
				{50u, 50u},
				{51u, 51u},
				{52u, 52u},
				{53u, 53u},
				{54u, 54u},
				{55u, 55u},
				{56u, 56u},
				{57u, 57u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{97u, 65u},
				{98u, 66u},
				{99u, 67u},
				{100u, 68u},
				{101u, 69u},
				{102u, 70u},
				{103u, 71u},
				{104u, 72u},
				{105u, 73u},
				{106u, 74u},
				{107u, 75u},
				{108u, 76u},
				{109u, 77u},
				{110u, 78u},
				{111u, 79u},
				{112u, 80u},
				{113u, 81u},
				{114u, 82u},
				{115u, 83u},
				{116u, 84u},
				{117u, 85u},
				{118u, 86u},
				{119u, 87u},
				{120u, 88u},
				{121u, 89u},
				{122u, 90u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{192u, 192u},
				{188u, 188u},
				{189u, 189u},
				{190u, 190u},
				{185u, 185u},
				{186u, 186u},
				{187u, 187u},
				{182u, 182u},
				{183u, 183u},
				{184u, 184u},
				{198u, 198u},
				{196u, 196u},
				{0u, 0u},
				{195u, 195u},
				{193u, 193u},
				{194u, 194u},
				{167u, 167u},
				{168u, 168u},
				{169u, 169u},
				{170u, 170u},
				{171u, 171u},
				{172u, 172u},
				{173u, 173u},
				{174u, 174u},
				{175u, 175u},
				{176u, 176u},
				{177u, 177u},
				{178u, 178u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{0u, 0u},
				{197u, 197u},
				{0u, 0u}};

		const unsigned __int8 extendedVirtualKeyConvert[37] =
			{
				59u,
				61u,
				44u,
				45u,
				46u,
				47u,
				96u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				0u,
				91u,
				92u,
				93u,
				39u};

		bool xenonDevKeyIsDown[256] =
			{
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false};

		const int modifierKeyCodes[3] = {159, 158, 160};

		const unsigned __int16 modifierMasks[3] = {16u, 32u, 8u};

		int lastKeyDown = 0;
		int lastKeyPressTime = 0;
		char lastCharKeyDown = '\0';

		int IN_XenonDevKeyboardConvertKey(int vkKey)
		{
			if ((unsigned int)(vkKey - 1) <= 0x90)
				return tolower(virtualKeyConvert[vkKey][1]);
			if ((unsigned int)(vkKey - 186) > 0x24)
				return 0;
			return extendedVirtualKeyConvert[vkKey - 186];
		}

		void IN_XenonDevKeyboardProcessKeyRepeat(
			int portIndex,
			int __formal,
			char charKey,
			int convertedKey,
			unsigned int timeMsec)
		{
			char v7; // r29

			v7 = lastCharKeyDown;
			if (convertedKey)
				CL_GamepadButtonEventForPort(portIndex, convertedKey, 1, timeMsec);
			// if (v7)
			// 	Sys_QueEvent(timeMsec, SE_CHAR, v7, 0, portIndex, 0, 0);
		}

		void IN_XenonDevKeyboardHandleModifiers(int portIndex, unsigned __int16 flags)
		{
			unsigned int v4;			// r25
			const unsigned __int16 *v5; // r30
			int *v6;					// r29
			int v7;						// r27
			int v8;						// r4
			bool v9;					// r11

			v4 = Sys_Milliseconds();
			v5 = modifierMasks;
			v6 = (int *)modifierKeyCodes;
			v7 = 3;
			do
			{
				v8 = *v6;
				v9 = (_CountLeadingZeros((unsigned __int16)(*v5 & flags)) & 0x20) == 0;
				if (v9 != xenonDevKeyIsDown[*v6])
				{
					xenonDevKeyIsDown[v8] = v9;
					CL_GamepadButtonEventForPort(portIndex, v8, v9, v4);
				}
				--v7;
				++v5;
				++v6;
			} while (v7);
		}

		void IN_XenonDevKeyboard()
		{
			int v0;							  // r26
			int v1;							  // r27
			char v2;						  // r11
			unsigned int VirtualKey;		  // r31
			int v4;							  // r29
			char Unicode;					  // r11
			char v6;						  // r30
			int v7;							  // r11
			int LocalClientNumForEventOnPort; // r3
			int v9;							  // r3
			_XINPUT_KEYSTROKE v10[14];		  // [sp+50h] [-70h] BYREF

			v0 = Sys_Milliseconds();
			v1 = *cl_controller_in_use;
			// if (!s_gamePads[0].enabled || (v2 = 1, !s_gamePads[0].keyboardEnabled))
			// 	v2 = 0;
			v2 = 1;
			if (v2)
			{
				while (!XInputGetKeystroke(0, 2u, v10))
				{
					if (!v10[0].Flags)
						break;
					VirtualKey = v10[0].VirtualKey;
					if ((unsigned int)v10[0].VirtualKey - 1 > 0x90)
					{
						if ((unsigned int)v10[0].VirtualKey - 186 > 0x24)
							v4 = 0;
						else
							v4 = extendedVirtualKeyConvert[v10[0].VirtualKey - 186];
					}
					else
					{
						v4 = tolower(virtualKeyConvert[v10[0].VirtualKey][1]);
					}
					if (VirtualKey <= 0xFF)
					{
						IN_XenonDevKeyboardHandleModifiers(0, v10[0].Flags);
						if ((v10[0].Flags & 1) != 0 && !xenonDevKeyIsDown[VirtualKey])
						{
#pragma warning(push)
#pragma warning(disable : 4244)
							Unicode = v10[0].Unicode;
#pragma warning(pop)

							if (v10[0].Unicode > 0x7Fu)
								Unicode = 0;
							v6 = Unicode;
							xenonDevKeyIsDown[VirtualKey] = 1;
							if (v4)
								CL_GamepadButtonEventForPort(v1, v4, 1, v0);
							// if (v6)
							// 	Sys_QueEvent(v0, SE_CHAR, v6, 0, v1, 0, 0);
							lastKeyDown = VirtualKey;
							lastCharKeyDown = v6;
							lastKeyPressTime = v0;
						}
						if ((v10[0].Flags & 2) != 0 && xenonDevKeyIsDown[VirtualKey])
						{
							v7 = *cl_multi_gamepads_enabled;
							*cl_last_controller_input = v1;
							xenonDevKeyIsDown[VirtualKey] = 0;
							if (v7 || v1 == *cl_controller_in_use)
							{
								LocalClientNumForEventOnPort = Com_GetLocalClientNumForEventOnPort(v1);
								CL_GamepadButtonEvent(LocalClientNumForEventOnPort, v1, v4, 0, v0);
							}
						}
					}
				}
				if (lastKeyDown && xenonDevKeyIsDown[lastKeyDown] && v0 - lastKeyPressTime > 500)
				{
					lastKeyPressTime += 50;
					v9 = IN_XenonDevKeyboardConvertKey(lastKeyDown);
					IN_XenonDevKeyboardProcessKeyRepeat(v1, lastKeyDown, lastCharKeyDown, v9, v0);
				}
			}
		}

		Detour IN_GamepadsMove_Detour;

		void IN_GamepadsMove_Hook()
		{
			// DbgPrint("IN_GamepadsMove_Hook\n");
			IN_XenonDevKeyboard();
			IN_GamepadsMove_Detour.GetOriginal<decltype(&IN_GamepadsMove_Hook)>()();
		}

		Detour CL_GamepadButtonEvent_Detour;

		void CL_GamepadButtonEvent_Hook(int localClientNum, int controllerIndex, int key, int down, int time)
		{
			DbgPrint("CL_GamepadButtonEvent_Hook: %d %d %d %d %d\n", localClientNum, controllerIndex, key, down, time);

			CL_GamepadButtonEvent_Detour.GetOriginal<decltype(&CL_GamepadButtonEvent_Hook)>()(localClientNum, controllerIndex, key, down, time);

			// CL_GamepadButtonEvent(localClientNum, controllerIndex, key, down, time);
		}

		void init()
		{

			IN_GamepadsMove_Detour = Detour(IN_GamepadsMove, IN_GamepadsMove_Hook);
			IN_GamepadsMove_Detour.Install();

			CL_GamepadButtonEvent_Detour = Detour(CL_GamepadButtonEvent, CL_GamepadButtonEvent_Hook);
			CL_GamepadButtonEvent_Detour.Install();

			XNotifyQueueUI(0, 0, XNOTIFY_SYSTEM, L"iw3 xenon mp", nullptr);
			Sleep(5000);

			// Opens the console.
			Key_SetCatcher(0, 0x1);
			con->outputVisible = true;

			// hello
			CL_ConsoleCharEvent(0, 104);
			CL_ConsoleCharEvent(0, 101);
			CL_ConsoleCharEvent(0, 108);
			CL_ConsoleCharEvent(0, 108);
			CL_ConsoleCharEvent(0, 111);
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
