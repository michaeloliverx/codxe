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
		Cbuf_AddText_t Cbuf_AddText = reinterpret_cast<Cbuf_AddText_t>(0x82239FD0);
		CL_CharEvent_t CL_CharEvent = reinterpret_cast<CL_CharEvent_t>(0x822D77F8);
		CL_ConsoleCharEvent_t CL_ConsoleCharEvent = reinterpret_cast<CL_ConsoleCharEvent_t>(0x822D7368);
		CL_GamepadButtonEvent_t CL_GamepadButtonEvent = reinterpret_cast<CL_GamepadButtonEvent_t>(0x822DD1E8);
		CL_GamepadButtonEventForPort_t CL_GamepadButtonEventForPort = reinterpret_cast<CL_GamepadButtonEventForPort_t>(0x822DD910);
		Com_GetLocalClientNumForEventOnPort_t Com_GetLocalClientNumForEventOnPort = reinterpret_cast<Com_GetLocalClientNumForEventOnPort_t>(0x82235BD8);
		IN_GamepadsMove_t IN_GamepadsMove = reinterpret_cast<IN_GamepadsMove_t>(0x82352A50);
		Key_SetCatcher_t Key_SetCatcher = reinterpret_cast<Key_SetCatcher_t>(0x822D6420);
		Sys_Milliseconds_t Sys_Milliseconds = reinterpret_cast<Sys_Milliseconds_t>(0x821A3C70);
		Con_CancelAutoComplete_t Con_CancelAutoComplete = reinterpret_cast<Con_CancelAutoComplete_t>(0x822DF3B0);
		Con_Close_t Con_Close = reinterpret_cast<Con_Close_t>(0x822DFC38);
		Com_Printf_t Com_Printf = reinterpret_cast<Com_Printf_t>(0x82237000);
		Com_sprintf_t Com_sprintf = reinterpret_cast<Com_sprintf_t>(0x821CCED8);
		Dvar_SetStringFromSource_t Dvar_SetStringFromSource = reinterpret_cast<Dvar_SetStringFromSource_t>(0x821D4148);
		I_strnicmp_t I_strnicmp = reinterpret_cast<I_strnicmp_t>(0x821CDA98);
		UI_KeyEvent_t UI_KeyEvent = reinterpret_cast<UI_KeyEvent_t>(0x821EF510);
		UI_SetActiveMenu_t UI_SetActiveMenu = reinterpret_cast<UI_SetActiveMenu_t>(0x821EF880);
		Scoreboard_HandleInputNonXboxLive_t Scoreboard_HandleInputNonXboxLive = reinterpret_cast<Scoreboard_HandleInputNonXboxLive_t>(0x822FEF80);
		Scoreboard_HandleInputXboxLive_t Scoreboard_HandleInputXboxLive = reinterpret_cast<Scoreboard_HandleInputXboxLive_t>(0x822FED48);
		CL_StopLogoOrCinematic_t CL_StopLogoOrCinematic = reinterpret_cast<CL_StopLogoOrCinematic_t>(0x822D0298);
		CL_DisconnectLocalClient_t CL_DisconnectLocalClient = reinterpret_cast<CL_DisconnectLocalClient_t>(0x822D03D0);
		CL_AllLocalClientsDisconnected_t CL_AllLocalClientsDisconnected = reinterpret_cast<CL_AllLocalClientsDisconnected_t>(0x822CF5C0);
		SCR_UpdateScreen_t SCR_UpdateScreen = reinterpret_cast<SCR_UpdateScreen_t>(0x822CA3E8);
		Field_AdjustScroll_t Field_AdjustScroll = reinterpret_cast<Field_AdjustScroll_t>(0x822D6E70);
		Con_AnySpaceAfterCommand_t Con_AnySpaceAfterCommand = reinterpret_cast<Con_AnySpaceAfterCommand_t>(0x822DEEB0);
		// Menu_HandleKey_t Menu_HandleKey = reinterpret_cast<Menu_HandleKey_t>(0x821E5260);	// unused
		Menu_GetFocused_t Menu_GetFocused = reinterpret_cast<Menu_GetFocused_t>(0x821DE8A8);
		Sys_GetEvent_t Sys_GetEvent = reinterpret_cast<Sys_GetEvent_t>(0x821AA1F0);
		CL_Input_t CL_Input = reinterpret_cast<CL_Input_t>(0x822DCBC8);

		// Variables
		auto con = reinterpret_cast<Console *>(0x82407590);
		auto conDrawInputGlob = reinterpret_cast<ConDrawInputGlob *>(0x82406FB0);
		auto cl_controller_in_use = reinterpret_cast<int *>(0x850274B8);
		auto cl_last_controller_input = reinterpret_cast<int *>(0x850274BC);
		auto cl_multi_gamepads_enabled = reinterpret_cast<int *>(0x823A46E4);
		auto cl_waitingOnServerToLoadMap = reinterpret_cast<bool *>(0x824359FC);
		auto s_gamePads = reinterpret_cast<GamePad *>(0x84C00DB8);
		auto clientUIActives = reinterpret_cast<clientUIActive_t *>(0x82435A10);
		auto playerKeys = reinterpret_cast<PlayerKeyState *>(0x8242AC5C);
		auto g_consoleField = reinterpret_cast<field_t *>(0x8242AA18);
		auto g_editingField = reinterpret_cast<field_t *>(0x85027540);
		auto con_restricted = reinterpret_cast<dvar_s *>(0x82406ED4);
		auto com_sv_running = reinterpret_cast<dvar_s *>(0x82A7F744);
		auto onlinegame = reinterpret_cast<dvar_s *>(0x824359D0);
		auto cls = reinterpret_cast<clientStatic_t *>(0x824303A0);
		auto scrPlaceFull = reinterpret_cast<ScreenPlacement *>(0x8246F420);
		auto uiInfoArray = reinterpret_cast<uiInfo_s *>(0x849F4288);
		auto eventHead = *reinterpret_cast<int *>(0x84C2A5B4);
		auto eventTail = *reinterpret_cast<int *>(0x84C2C998);
		auto eventQue = reinterpret_cast<sysEvent_t *>(0x84C2AD90);
		cmd_function_s *cmd_functions = reinterpret_cast<cmd_function_s *>(0x82A2335C);

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

		// void Sys_QueEvent(int time, sysEventType_t type, int value, int value2, int portIndex, int ptrLength, void *ptr)
		// {
		// 	DbgPrint("Sys_QueEvent: type: %d, value: %d, portIndex: %d\n", type, value, portIndex);
		// 	// Check if event is SE_CHAR
		// 	if (type == SE_CHAR)
		// 	{
		// 		DbgPrint("CHARACTER INPUT QUEUED: %c (value: %d)\n", (char)value, value);
		// 	}
		// 	int v8;			 // r11
		// 	sysEvent_t *v11; // r30

		// 	v8 = eventHead;
		// 	v11 = &eventQue[eventHead & 0xFF];
		// 	if (eventHead - eventTail >= 256)
		// 	{
		// 		Com_Printf(16, "Sys_QueEvent: overflow\n");
		// 		++eventTail;
		// 		v8 = eventHead;
		// 	}
		// 	eventHead = v8 + 1;
		// 	if (!time)
		// 		time = Sys_Milliseconds();
		// 	v11->evTime = time;
		// 	v11->evValue = value;
		// 	v11->evPortIndex = portIndex;
		// 	v11->evValue2 = 0;
		// 	v11->evType = SE_CHAR;
		// 	v11->evPtrLength = 0;
		// 	v11->evPtr = 0;
		// }

		void Sys_QueEvent(int time, sysEventType_t type, int value, int value2, int portIndex, int ptrLength, void *ptr)
		{
			static int &eventHead = *reinterpret_cast<int *>(0x84C2A5B4);
			static int &eventTail = *reinterpret_cast<int *>(0x84C2C998);
			static sysEvent_t *eventQue = reinterpret_cast<sysEvent_t *>(0x84C2AD90);

			DbgPrint("Sys_QueEvent: type: %d, value: %d, portIndex: %d, eventHead=%d, eventTail=%d\n",
					 type, value, portIndex, eventHead, eventTail);

			if (type == SE_CHAR)
			{
				DbgPrint("CHARACTER INPUT QUEUED: %c (value: %d)\n", (char)value, value);
			}

			if (eventHead - eventTail >= 256)
			{
				Com_Printf(16, "Sys_QueEvent: overflow\n");
				eventTail++; // Ensure eventTail moves forward to prevent an infinite loop.
			}

			sysEvent_t *event = &eventQue[eventHead & 0xFF];

			if (!time)
				time = Sys_Milliseconds();

			event->evTime = time;
			event->evValue = value;
			event->evPortIndex = portIndex;
			event->evValue2 = value2;
			event->evType = type;
			event->evPtrLength = 0;
			event->evPtr = 0;

			// Ensure eventHead is properly updated.
			eventHead++;
			*reinterpret_cast<int *>(0x84C2A5B4) = eventHead;
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
			if (v7)
				Sys_QueEvent(timeMsec, SE_CHAR, v7, 0, portIndex, 0, 0);
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
							if (v6)
								Sys_QueEvent(v0, SE_CHAR, v6, 0, v1, 0, 0);
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

		int g_console_field_width = 620;
		float g_console_char_height = 18.0;

		void Con_ToggleConsole()
		{
			DbgPrint("Con_ToggleConsole\n");
			char v0; // r10

			memset(g_consoleField->buffer, 0, sizeof(g_consoleField->buffer));
			g_consoleField->cursor = 0;
			g_consoleField->scroll = 0;
			g_consoleField->drawWidth = 256;
			if (conDrawInputGlob->matchIndex < 0 || (v0 = 1, !conDrawInputGlob->autoCompleteChoice[0]))
				v0 = 0;
			if (v0)
			{
				conDrawInputGlob->matchIndex = -1;
				conDrawInputGlob->autoCompleteChoice[0] = 0;
			}
			g_consoleField->widthInPixels = g_console_field_width;
			g_consoleField->charHeight = g_console_char_height;
			g_consoleField->fixedSize = 1;
			con->outputVisible = 0;
			clientUIActives[0].keyCatchers ^= 1u;
			clientUIActives[1].keyCatchers ^= 1u;
			clientUIActives[2].keyCatchers ^= 1u;
			clientUIActives[3].keyCatchers ^= 1u;
		}

		void Con_ToggleConsoleOutput()
		{
			con->outputVisible = !con->outputVisible;
		}

		int IsGamepadKey(int key)
		{
			bool v1;	// cr58
			int result; // r3

			switch (key)
			{
			case 1:
				return 1;
			case 2:
				return 1;
			case 3:
				return 1;
			case 4:
				return 1;
			case 14:
				return 1;
			case 15:
				return 1;
			case 5:
				return 1;
			case 6:
				return 1;
			case 16:
				return 1;
			case 17:
				return 1;
			case 18:
				return 1;
			case 19:
				return 1;
			case 20:
				return 1;
			case 21:
				return 1;
			case 22:
				return 1;
			case 23:
				return 1;
			case 28:
				return 1;
			case 29:
				return 1;
			case 30:
				return 1;
			}
			v1 = key == 31;
			result = 0;
			if (v1)
				return 1;
			return result;
		}

		char tinystr[5] = {'\0', '\0', '\0', '\0', '\0'};

		keyname_t keynames[116] =
			{
				{"TAB", 9},
				{"ENTER", 13},
				{"ESCAPE", 27},
				{"SPACE", 32},
				{"BACKSPACE", 127},
				{"UPARROW", 154},
				{"DOWNARROW", 155},
				{"LEFTARROW", 156},
				{"RIGHTARROW", 157},
				{"ALT", 158},
				{"CTRL", 159},
				{"SHIFT", 160},
				{"CAPSLOCK", 151},
				{"F1", 167},
				{"F2", 168},
				{"F3", 169},
				{"F4", 170},
				{"F5", 171},
				{"F6", 172},
				{"F7", 173},
				{"F8", 174},
				{"F9", 175},
				{"F10", 176},
				{"F11", 177},
				{"F12", 178},
				{"INS", 161},
				{"DEL", 162},
				{"PGDN", 163},
				{"PGUP", 164},
				{"HOME", 165},
				{"END", 166},
				{"MOUSE1", 200},
				{"MOUSE2", 201},
				{"MOUSE3", 202},
				{"MOUSE4", 203},
				{"MOUSE5", 204},
				{"MWHEELUP", 206},
				{"MWHEELDOWN", 205},
				{"BUTTON_A", 1},
				{"BUTTON_B", 2},
				{"BUTTON_X", 3},
				{"BUTTON_Y", 4},
				{"BUTTON_LSHLDR", 5},
				{"BUTTON_RSHLDR", 6},
				{"BUTTON_START", 14},
				{"BUTTON_BACK", 15},
				{"BUTTON_LSTICK", 16},
				{"BUTTON_RSTICK", 17},
				{"BUTTON_RTRIG", 19},
				{"BUTTON_LTRIG", 18},
				{"DPAD_UP", 20},
				{"DPAD_DOWN", 21},
				{"DPAD_LEFT", 22},
				{"DPAD_RIGHT", 23},
				{"APAD_UP", 28},
				{"APAD_DOWN", 29},
				{"APAD_LEFT", 30},
				{"APAD_RIGHT", 31},
				{"AUX1", 207},
				{"AUX2", 208},
				{"AUX3", 209},
				{"AUX4", 210},
				{"AUX5", 211},
				{"AUX6", 212},
				{"AUX7", 213},
				{"AUX8", 214},
				{"AUX9", 215},
				{"AUX10", 216},
				{"AUX11", 217},
				{"AUX12", 218},
				{"AUX13", 219},
				{"AUX14", 220},
				{"AUX15", 221},
				{"AUX16", 222},
				{"KP_HOME", 182},
				{"KP_UPARROW", 183},
				{"KP_PGUP", 184},
				{"KP_LEFTARROW", 185},
				{"KP_5", 186},
				{"KP_RIGHTARROW", 187},
				{"KP_END", 188},
				{"KP_DOWNARROW", 189},
				{"KP_PGDN", 190},
				{"KP_ENTER", 191},
				{"KP_INS", 192},
				{"KP_DEL", 193},
				{"KP_SLASH", 194},
				{"KP_MINUS", 195},
				{"KP_PLUS", 196},
				{"KP_NUMLOCK", 197},
				{"KP_STAR", 198},
				{"KP_EQUALS", 199},
				{"PAUSE", 153},
				{"SEMICOLON", 59},
				{"COMMAND", 150},
				{"181", 128},
				{"191", 129},
				{"223", 130},
				{"224", 131},
				{"225", 132},
				{"228", 133},
				{"229", 134},
				{"230", 135},
				{"231", 136},
				{"232", 137},
				{"233", 138},
				{"236", 139},
				{"241", 140},
				{"242", 141},
				{"243", 142},
				{"246", 143},
				{"248", 144},
				{"249", 145},
				{"250", 146},
				{"252", 147},
				{NULL, 0},
		};

		void Message_Key(int localClientNum, int key)
		{
			DbgPrint("Message_Key: localClientNum: %d, key: %d\n", localClientNum, key);
		}

		int Console_IsRconCmd(const char *commandString)
		{
			const char *v1;	 // r11
			const char *v2;	 // r10
			int v4;			 // r10
			bool v5;		 // cr58
			int v6;			 // r9
			int v7;			 // r8
			unsigned int v9; // r11

			v1 = "rcon";
			v2 = "rcon";
			while (*(unsigned __int8 *)v2++)
				;
			v4 = v2 - "rcon" - 1;
#pragma warning(push)
#pragma warning(disable : 4127)
			while (1)
#pragma warning(pop)
			{
				v5 = v4-- == 0;
				v6 = v1[commandString - "rcon"];
				v7 = *v1++;
				if (v5)
					return 1;
				if (v6 != v7)
					break;
				if (!v6)
					return 1;
			}
			if (v6 >= v7)
				v9 = _CountLeadingZeros(1u);
			else
				v9 = _CountLeadingZeros(0xFFFFFFFF);
			return (v9 >> 5) & 1;
		}

		void Con_PageUp()
		{
			con->displayLineOffset -= 2;
			if (con->displayLineOffset < con->visibleLineCount)
			{
				if (con->consoleWindow.activeLineCount < con->visibleLineCount)
					con->displayLineOffset = con->consoleWindow.activeLineCount;
				else
					con->displayLineOffset = con->visibleLineCount;
			}
		}

		void Con_PageDown()
		{
			if (con->displayLineOffset + 2 < con->consoleWindow.activeLineCount)
				con->displayLineOffset += 2;
			else
				con->displayLineOffset = con->consoleWindow.activeLineCount;
		}

		bool Console_IsScrollDownKey(int key, int isCtrlDown)
		{
			return key == 189 || tolower(key) == 110 && isCtrlDown;
		}

		bool Console_IsScrollUpKey(int key, int isCtrlDown)
		{
			return key == 183 || tolower(key) == 112 && isCtrlDown;
		}

		int Con_CycleAutoComplete(int step)
		{
			int matchCount; // r10
			int v3;			// r11
			int result;		// r3

			if (!conDrawInputGlob->mayAutoComplete)
				return 0;
			matchCount = conDrawInputGlob->matchCount;
			if ((unsigned int)(conDrawInputGlob->matchCount - 2) > 0x15)
				return 0;
			if (!conDrawInputGlob->hasExactMatch)
				goto LABEL_6;
			if (Con_AnySpaceAfterCommand())
				return 0;
			matchCount = conDrawInputGlob->matchCount;
		LABEL_6:
			v3 = conDrawInputGlob->matchIndex + step;
			conDrawInputGlob->matchIndex = v3;
			if (v3 >= 0)
			{
				if (v3 >= matchCount)
					conDrawInputGlob->matchIndex = 0;
				return 1;
			}
			else
			{
				result = 1;
				conDrawInputGlob->matchIndex = matchCount - 1;
			}
			return result;
		}

		void CompleteCommand()
		{
			DbgPrint("CompleteCommand\n");
		}

		int Con_CommitToAutoComplete()
		{
			DbgPrint("Con_CommitToAutoComplete\n");
			return 0;
		}

		int Field_KeyDownEvent(int localClientNum, const ScreenPlacement *scrPlace, field_t *edit, int key)
		{
			// DbgPrint("Field_KeyDownEvent: localClientNum: %d, key: %d\n", localClientNum, key);

			int v4;		  // r29
			char *buffer; // r11
			int down;	  // r26
			int v11;	  // r11
			int v12;	  // r27
			int v13;	  // r11
			int v14;	  // r11
			int v15;	  // r11

			v4 = localClientNum;
			buffer = edit->buffer;
			down = playerKeys[localClientNum].keys[159].down;
			while (*buffer++)
				;
			v11 = buffer - edit->buffer - 1;
			v12 = v11;
			if (key != 161 && key != 192 || !playerKeys[localClientNum].keys[160].down)
			{
				switch (key)
				{
				case 162:
					if (edit->cursor < v11)
						memmove(&edit->buffer[edit->cursor], &edit->buffer[edit->cursor + 1], v11 - edit->cursor);
					break;
				case 157:
					if (edit->cursor < v11)
						++edit->cursor;
					if (down && edit->cursor < v11)
					{
						do
						{
							if (!isalnum(edit->buffer[edit->cursor]))
								break;
							v13 = edit->cursor + 1;
							edit->cursor = v13;
						} while (v13 < v12);
						if (edit->cursor < v12)
						{
							do
							{
								if (isalnum(edit->buffer[edit->cursor]))
									break;
								v14 = edit->cursor + 1;
								edit->cursor = v14;
							} while (v14 < v12);
						}
					}
					break;
				case 156:
					if (edit->cursor > 0)
						--edit->cursor;
					if (down && edit->cursor > 0)
					{
						do
						{
							if (!isalnum(*((char *)&edit->fixedSize + edit->cursor + 3)))
								break;
							v15 = edit->cursor - 1;
							edit->cursor = v15;
						} while (v15 > 0);
					}
					if (edit->cursor < edit->scroll)
						edit->scroll = edit->cursor;
					break;
				default:
					if (key == 165 || tolower(key) == 97 && down)
					{
						edit->cursor = 0;
					}
					else if (key == 166 || tolower(key) == 101 && down)
					{
						edit->cursor = v12;
					}
					else if (key == 161)
					{
						playerKeys[v4].overstrikeMode = playerKeys[v4].overstrikeMode == 0;
					}
					break;
				}
			}
			if (cls->uiStarted)
				Field_AdjustScroll(scrPlace, edit);
			return 0;
		}

		bool s_shouldCompleteCmd = false;

		int historyLine = 0;
		int nextHistoryLine = 0;
		field_t historyEditLines[32] =
			{
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
				{0, 0, 0, 0, 0.0, 0, ""},
		};

		void Console_Key(int localClientNum, int key)
		{
			DbgPrint("Console_Key: localClientNum: %d, key: %d\n", localClientNum, key);

			BOOL v4;			 // r9
			int down;			 // r27
			int v6;				 // r28
			int v7;				 // r26
			bool IsScrollUpKey;	 // r3
			int v10;			 // r11
			int activeLineCount; // r10
			// int v13;			 // r8
			char *buffer; // r4
			int v15;	  // r30

			v4 = s_shouldCompleteCmd;
			down = playerKeys[localClientNum].keys[160].down;
			v6 = playerKeys[localClientNum].keys[159].down;
			v7 = playerKeys[localClientNum].keys[158].down;
			s_shouldCompleteCmd = 1;
			if (key == 108)
			{
				if (v6)
				{
					Cbuf_AddText(localClientNum, "clear\n");
					return;
				}
				goto LABEL_17;
			}
			if (key != 13 && key != 191)
			{
				if (key == 9)
				{
					if (v4)
						CompleteCommand();
					else
						Con_CycleAutoComplete(down == 0 ? 1 : -1);
					s_shouldCompleteCmd = 0;
					return;
				}
				if (key == 154)
				{
					if (v6)
					{
						Con_CycleAutoComplete(-1);
						return;
					}
					IsScrollUpKey = 1;
				LABEL_19:
					if (IsScrollUpKey)
					{
						v10 = historyLine;
						if (nextHistoryLine - historyLine < 32 && historyLine > 0)
							v10 = --historyLine;
						memcpy(&g_consoleField, &historyEditLines[v10 % 32], sizeof(g_consoleField));
						Field_AdjustScroll(scrPlaceFull, g_consoleField);
						conDrawInputGlob->mayAutoComplete = 0;
						return;
					}
					if (key == 155 || Console_IsScrollDownKey(key, v6))
					{
						if (!Con_CycleAutoComplete(1) && historyLine != nextHistoryLine)
						{
							memcpy(&g_consoleField, &historyEditLines[++historyLine % 32], sizeof(g_consoleField));
							Field_AdjustScroll(scrPlaceFull, g_consoleField);
						}
						return;
					}
					switch (key)
					{
					case 164:
						Con_PageUp();
						return;
					case 163:
						Con_PageDown();
						return;
					case 165:
						if (v6)
						{
							activeLineCount = con->consoleWindow.activeLineCount;
							if (con->consoleWindow.activeLineCount >= con->visibleLineCount)
							{
								con->displayLineOffset = con->visibleLineCount;
								return;
							}
							goto LABEL_42;
						}
						break;
					case 166:
						if (v6)
						{
							activeLineCount = con->consoleWindow.activeLineCount;
						LABEL_42:
							con->displayLineOffset = activeLineCount;
							return;
						}
						break;
					case 162:
					case 27:
						if ((unsigned __int8)Con_CancelAutoComplete())
							return;
						goto LABEL_55;
					case 157:
					case 187:
					case 156:
					case 185:
						goto LABEL_53;
					default:
						if (key == 127 || v6)
						{
						LABEL_55:
							if (Field_KeyDownEvent(localClientNum, scrPlaceFull, g_consoleField, key))
								conDrawInputGlob->mayAutoComplete = 1;
							return;
						}
						break;
					}
					if (!v7 && !down)
					{
					LABEL_53:
						Con_CommitToAutoComplete();
						goto LABEL_55;
					}
					goto LABEL_55;
				}
				if (key == 155 && v6)
				{
					Con_CycleAutoComplete(1);
					return;
				}
			LABEL_17:
				IsScrollUpKey = Console_IsScrollUpKey(key, v6);
				goto LABEL_19;
			}
			if (Con_CommitToAutoComplete())
				return;
			Com_Printf(0, "]%s\n", g_consoleField->buffer);
			if (g_consoleField->buffer[0] == 92 || g_consoleField->buffer[0] == 47)
			{
				buffer = &g_consoleField->buffer[1];
			}
			else if (I_strnicmp(g_consoleField->buffer, "quit", 4) && I_strnicmp(g_consoleField->buffer, "kill", 4))
			{
				buffer = g_consoleField->buffer;
			}
			else
			{
				// if (!v13)
				// 	return;
				if (Console_IsRconCmd(g_consoleField->buffer))
					goto LABEL_68;
				Cbuf_AddText(localClientNum, "cmd say ");
				buffer = g_consoleField->buffer;
			}
			Cbuf_AddText(localClientNum, buffer);
			Cbuf_AddText(localClientNum, "\n");
		LABEL_68:
			if (g_consoleField->buffer[0])
			{
				v15 = nextHistoryLine;
				memcpy(&historyEditLines[nextHistoryLine % 32], &g_consoleField, sizeof(field_t));
				nextHistoryLine = v15 + 1;
				historyLine = v15 + 1;
			}
			memset(g_consoleField->buffer, 0, sizeof(g_consoleField->buffer));
			g_consoleField->cursor = 0;
			g_consoleField->scroll = 0;
			g_consoleField->charHeight = g_console_char_height;
			g_consoleField->drawWidth = 256;
			g_consoleField->widthInPixels = g_console_field_width;
			g_consoleField->fixedSize = 1;
			if (CL_AllLocalClientsDisconnected())
				SCR_UpdateScreen();
		}

		int UI_CheckExecKey(int localClientNum, int key)
		{
			// int v2;				   // r4
			menuDef_t *Focused;	   // r3
			ItemKeyHandler *onKey; // r11

			Focused = Menu_GetFocused(&uiInfoArray[localClientNum].uiDC);
			if (g_editingField)
				return 1;
			if (key <= 256)
			{
				if (Focused)
				{
					onKey = Focused->onKey;
					if (onKey)
					{
						while (onKey->key != key)
						{
							onKey = onKey->next;
							if (!onKey)
								return 0;
						}
						return 1;
					}
				}
			}
			return 0;
		}

		// TODO: get global reference
		auto cl_bypassMouseInput = 0;

		int CL_MouseInputShouldBypassMenus(int localClientNum, int key)
		{
			int result; // r3
			bool v3;	// cr58

			if (uiInfoArray[localClientNum].currentMenuType == UIMENU_SCOREBOARD)
				return 1;
			if (!cl_bypassMouseInput)
				return 0;
			// if (!cl_bypassMouseInput->current.enabled)
			// 	return 0;
			if (!cls->uiStarted)
				return 0;
			v3 = UI_CheckExecKey(localClientNum, key) == 0;
			result = 1;
			if (!v3)
				return 0;
			return result;
		}

#pragma warning(push)
#pragma warning(disable : 4102)
#pragma warning(disable : 4101)
#pragma warning(disable : 4244)
#pragma warning(disable : 4700)

		void CL_KeyEvent(int localClientNum, int key, int down, unsigned int time)
		{
			DbgPrint("CL_KeyEvent: localClientNum: %d, key: %d, down: %d, time: %d\n", localClientNum, key, down, time);
			PlayerKeyState *v5;					  // r6
			int v7;								  // r29
			int v8;								  // r31
			KeyState *keys;						  // r23
			KeyState *v11;						  // r7
			int v12;							  // r11
			int v13;							  // r10
			int v14;							  // r26
			clientUIActive_t *v15;				  // r24
			const char *v16;					  // r8
			int keyCatchers;					  // r11
			int v18;							  // r10
			LocSelInputState *p_locSelInputState; // r6
			const char *binding;				  // r11
			const char *v21;					  // r10
			int v22;							  // r8
			connstate_t connectionState;		  // r29
			int v24;							  // r9
			bool v25;							  // r3
			const char *v26;					  // r11
			int v27;							  // r5
			const char *v28;					  // r6
			char v29;							  // r3
			keyname_t *v30;						  // r11
			int v31;							  // r11
			char v32;							  // r9
			char v33;							  // r11
			char v34;							  // r11
			char v35[1120];						  // [sp+80h] [-460h] BYREF

			v5 = playerKeys;
			v7 = 842 * localClientNum;
			v8 = key;
			keys = playerKeys[localClientNum].keys;
			v11 = &keys[key];
			v11->down = down;
			if (down)
			{
				v12 = v11->repeats + 1;
				v11->repeats = v12;
				if (v12 == 1)
					++playerKeys[v7 / 0x34Au].anyKeyDown;
			}
			else
			{
				v11->repeats = 0;
				v13 = playerKeys[v7 / 0x34Au].anyKeyDown - 1;
				playerKeys[v7 / 0x34Au].anyKeyDown = v13;
				if (v13 < 0)
					playerKeys[v7 / 0x34Au].anyKeyDown = 0;
			}
			v14 = localClientNum;
			v15 = &clientUIActives[localClientNum];
			if (key != 96 && key != 126 && (v15->keyCatchers & 1) == 0 && v11->binding && !I_strnicmp(v11->binding, "devgui", 0x7FFFFFFF))
			{
				if (!down)
					goto LABEL_22;
				if (v11->repeats == 1)
				{
					Cbuf_AddText(localClientNum, v16);
					return;
				}
			}
			if (down && v11->repeats > 1)
			{
				keyCatchers = v15->keyCatchers;
				if ((keyCatchers & 0x21) == 0)
				{
					if ((keyCatchers & 0x10) != 0)
					{
						switch (v8)
						{
						case 154:
						case 155:
						case 163:
						case 164:
							goto LABEL_19;
						default:
							return;
						}
					}
					return;
				}
			LABEL_19:
				if (v8 == 96 || v8 == 126 || v8 == 27)
					return;
			}
		LABEL_22:
			if (!con_restricted->current.enabled || (v15->keyCatchers & 1) != 0)
			{
				if (v8 == 96 || v8 == 126)
				{
					if (!down)
						return;
					if (keys[160].down)
					{
						if ((clientUIActives[v14].keyCatchers & 1) == 0)
							Con_ToggleConsole();
						con->outputVisible = (_CountLeadingZeros(con->outputVisible) & 0x20) != 0;
						return;
					}
				LABEL_27:
					Con_ToggleConsole();
					return;
				}
			}
			else if (v8 == 165)
			{
				if (down && keys[127].down)
					goto LABEL_27;
			}
			else if (v8 == 96 || v8 == 126)
			{
				return;
			}
			v18 = v15->keyCatchers;
			p_locSelInputState = &v5->locSelInputState;
			if ((v18 & 8) != 0 && down > 0)
			{
				if (v8 == 27)
				{
					p_locSelInputState[v7] = LOC_SEL_INPUT_CANCEL;
				}
				else
				{
					binding = v11->binding;
					if (binding)
					{
						v21 = "+attack";
						do
						{
							v22 = *(unsigned __int8 *)binding - *(unsigned __int8 *)v21;
							if (!*binding)
								break;
							++binding;
							++v21;
						} while (!v22);
						if (!v22)
							p_locSelInputState[v7] = LOC_SEL_INPUT_CONFIRM;
					}
				}
				return;
			}
			p_locSelInputState[v7] = LOC_SEL_INPUT_NONE;
			connectionState = clientUIActives[v14].connectionState;
			if (down && v8 < 128 && (connectionState == CA_CINEMATIC || connectionState == CA_LOGO) && !v18)
			{
				// Dvar_SetStringFromSource(nextdemo, byte_8204B4F3, DVAR_SOURCE_INTERNAL);
				v8 = 27;
			}
			else if (v8 != 27)
			{
				goto LABEL_68;
			}
			if (down)
			{
				v24 = v15->keyCatchers;
				if ((v24 & 0x20) != 0)
				{
					Message_Key(localClientNum, 27);
				}
				else
				{
					if ((v24 & 1) != 0)
						Con_CancelAutoComplete();
					if ((v24 & 0x10) != 0)
					{
						UI_KeyEvent(localClientNum, 27, down);
					}
					else
					{
						switch (connectionState)
						{
						case CA_CINEMATIC:
						case CA_LOGO:
							CL_StopLogoOrCinematic(localClientNum);
							break;
						case CA_CONNECTING:
						case CA_CHALLENGING:
						case CA_CONNECTED:
							if (!com_sv_running->current.enabled && !clientUIActives[v14].mapPreloaded)
							{
								Com_Printf(14, "Client cancelled connecting to server\n");
								CL_DisconnectLocalClient(localClientNum);
							}
							break;
						case CA_ACTIVE:
							if (cl_waitingOnServerToLoadMap[localClientNum])
								goto LABEL_59;
							UI_SetActiveMenu(localClientNum, 2);
							break;
						default:
							if (cls->uiStarted)
							LABEL_59:
								UI_SetActiveMenu(localClientNum, 1);
							break;
						}
					}
				}
				return;
			}
		LABEL_68:
			if (v15->cgameInitialized && uiInfoArray[localClientNum].currentMenuType == UIMENU_SCOREBOARD)
			{
				if (!down)
				{
				LABEL_78:
					v26 = keys[v8].binding;
					if (v26 && *v26 == 43)
					{
						Com_sprintf(v35, 1024, "-%s %i %i\n", v26 + 1, v8, time);
						Cbuf_AddText(localClientNum, v35);
					}
					if ((v15->keyCatchers & 0x10) == 0)
						return;
				LABEL_82:
					if (cls->uiStarted)
						UI_KeyEvent(localClientNum, v8, down);
					return;
				}
				if (clientUIActives[v14].connectionState >= CA_ACTIVE)
				{
					if (onlinegame->current.enabled)
#pragma warning(push)
#pragma warning(disable : 4800)
						v25 = Scoreboard_HandleInputXboxLive(localClientNum, v8);
					else
						v25 = Scoreboard_HandleInputNonXboxLive(localClientNum, v8);
#pragma warning(pop)
				}
				else
				{
					v25 = 0;
				}
				if (v25)
					return;
			}
			if (!down)
				goto LABEL_78;
			v27 = v15->keyCatchers;
			if ((v27 & 1) != 0)
				goto LABEL_90;
			if ((v27 & 0x10) != 0 && !CL_MouseInputShouldBypassMenus(localClientNum, v8))
				goto LABEL_82;
			if ((v27 & 0x20) != 0)
			{
				Message_Key(localClientNum, v8);
				return;
			}
			if (connectionState == CA_DISCONNECTED)
			{
			LABEL_90:
				Console_Key(localClientNum, v8);
				return;
			}
			v28 = keys[v8].binding;
			if (v28)
			{
				if (*v28 == 43)
				{
					Com_sprintf(v35, 1024, "%s %i %i\n", v28, v8, time);
					Cbuf_AddText(localClientNum, v35);
				}
				else
				{
					Cbuf_AddText(localClientNum, v28);
					Cbuf_AddText(localClientNum, "\n");
				}
				return;
			}
			if (v8 < 207)
				return;
			if ((unsigned int)v8 > 0xFF)
			{
				Com_Printf(14, "%s is unbound, use controls menu to set.\n", "<OUT OF RANGE>");
				return;
			}
			if ((unsigned int)(v8 - 33) <= 0x5D)
			{
				v29 = toupper(v8);
				tinystr[1] = 0;
				tinystr[0] = v29;
			LABEL_106:
				Com_Printf(14, "%s is unbound, use controls menu to set.\n", tinystr);
				return;
			}
			v30 = keynames;
			if (!keynames[0].name)
			{
			LABEL_99:
				v31 = v8 >> 4;
				v32 = v8 & 0xF;
				tinystr[0] = 48;
				tinystr[1] = 120;
				if (v8 >> 4 <= 9)
					v33 = v31 + 48;
				else
					v33 = v31 + 87;
				tinystr[2] = v33;
				v34 = v32 + 87;
				if ((v8 & 0xFu) <= 9)
					v34 = v32 + 48;
				tinystr[3] = v34;
				tinystr[4] = 0;
				goto LABEL_106;
			}
			while (v8 != v30->keynum)
			{
				++v30;
				if (!v30->name)
					goto LABEL_99;
			}
			Com_Printf(14, "%s is unbound, use controls menu to set.\n", v30->name);
		}

#pragma warning(pop)

		// Detour CL_CharEvent_Detour;

		// void CL_CharEvent_Hook(int localClientNum, int key)
		// {
		// 	DbgPrint("CL_CharEvent_Hook: localClientNum: %d, key: %d\n", localClientNum, key);
		// 	CL_CharEvent_Detour.GetOriginal<decltype(&CL_CharEvent_Hook)>()(localClientNum, key);
		// }

		Detour IN_GamepadsMove_Detour;

		void IN_GamepadsMove_Hook()
		{
			DbgPrint("IN_GamepadsMove_Hook\n");
			// IN_XenonDevKeyboard();
			IN_GamepadsMove_Detour.GetOriginal<decltype(&IN_GamepadsMove_Hook)>()();
		}

		Detour CL_GamepadButtonEvent_Detour;

		void CL_GamepadButtonEvent_Hook(int localClientNum, int controllerIndex, int key, int down, unsigned int time)
		{
			DbgPrint("CL_GamepadButtonEvent_Hook: %d %d %d %d %d\n", localClientNum, controllerIndex, key, down, time);

			if (!IsGamepadKey(key))
			{
				DbgPrint("CL_GamepadButtonEvent_Hook: not gamepad key\n");
				CL_KeyEvent(localClientNum, key, down, time);
				return;
			}

			CL_GamepadButtonEvent_Detour.GetOriginal<decltype(&CL_GamepadButtonEvent_Hook)>()(localClientNum, controllerIndex, key, down, time);
		}

		Detour Sys_GetEvent_Detour;

		sysEvent_t Sys_GetEvent_Hook(sysEvent_t *result)
		{
			sysEvent_t event = Sys_GetEvent_Detour.GetOriginal<decltype(&Sys_GetEvent_Hook)>()(result);

			// // Log event details
			// DbgPrint("Sys_GetEvent_Hook: Type=%d, Value=%d, Value2=%d, PortIndex=%d, PtrLen=%d\n",
			// 		 event.evType, event.evValue, event.evValue2, event.evPortIndex, event.evPtrLength);

			// Check if it's a character event
			if (event.evType == SE_CHAR)
			{
				DbgPrint("CHARACTER EVENT: '%c' (ASCII: %d)\n", (char)event.evValue, event.evValue);
			}

			return event;
		}

		// Ingame tracking
		// TODO: Find all places where IN_XenonDevKeyboard is called
		Detour CL_Input_Detour;

		void CL_Input_Hook(int localClientNum)
		{
			if (clientUIActives[localClientNum].connectionState == CA_ACTIVE)
			{
				IN_XenonDevKeyboard();
			}
			DbgPrint("CL_Input_Hook: localClientNum: %d\n", localClientNum);
			CL_Input_Detour.GetOriginal<decltype(&CL_Input_Hook)>()(localClientNum);
		}

		void R_ImageDump_f()
		{
			Com_Printf(0, "R_ImageDump_f\n");
		}

		void init()
		{

			CL_Input_Detour = Detour(CL_Input, CL_Input_Hook);
			CL_Input_Detour.Install();

			// Sys_GetEvent_Detour = Detour(Sys_GetEvent, Sys_GetEvent_Hook);
			// Sys_GetEvent_Detour.Install();

			// CL_CharEvent_Detour = Detour(CL_CharEvent, CL_CharEvent_Hook);
			// CL_CharEvent_Detour.Install();

			// IN_GamepadsMove_Detour = Detour(IN_GamepadsMove, IN_GamepadsMove_Hook);
			// IN_GamepadsMove_Detour.Install();

			CL_GamepadButtonEvent_Detour = Detour(CL_GamepadButtonEvent, CL_GamepadButtonEvent_Hook);
			CL_GamepadButtonEvent_Detour.Install();

			XNotifyQueueUI(0, 0, XNOTIFY_SYSTEM, L"iw3 xenon mp", nullptr);

			// clientUIActives[0].keyCatchers = 1;

			// Sleep(5000);
			// // Open the console.
			// Key_SetCatcher(0, 0x1);
			// con->outputVisible = true;

			// Hide the console.
			// Key_SetCatcher(0, 0);
			// con->outputVisible = false;

			// // hello
			// CL_ConsoleCharEvent(0, 104);
			// CL_ConsoleCharEvent(0, 101);
			// CL_ConsoleCharEvent(0, 108);
			// CL_ConsoleCharEvent(0, 108);
			// CL_ConsoleCharEvent(0, 111);

			// Open and close the console.
			// Sleep(5000);
			// Con_ToggleConsole();
			// CL_ConsoleCharEvent(0, 104);
			// CL_ConsoleCharEvent(0, 101);
			// CL_ConsoleCharEvent(0, 108);
			// CL_ConsoleCharEvent(0, 108);
			// CL_ConsoleCharEvent(0, 111);
			// Sleep(5000);
			// Con_ToggleConsole();

			// Open the console.
			// Sleep(5000);
			// Con_ToggleConsole();
			// Con_ToggleConsoleOutput();
			// CL_CharEvent(0, 104);
			// CL_CharEvent(0, 101);
			// CL_CharEvent(0, 108);
			// CL_CharEvent(0, 108);
			// CL_CharEvent(0, 111);
			// // write meminfo
			// CL_ConsoleCharEvent(0, 109);
			// CL_ConsoleCharEvent(0, 101);
			// CL_ConsoleCharEvent(0, 109);
			// CL_ConsoleCharEvent(0, 105);
			// CL_ConsoleCharEvent(0, 110);
			// CL_ConsoleCharEvent(0, 102);
			// CL_ConsoleCharEvent(0, 111);

			// cmd_function_s *cmd = new cmd_function_s;
			// cmd->name = "qwerty";
			// cmd->autoCompleteDir = "autoCompleteDir\n";
			// cmd->autoCompleteExt = "autoCompleteExt\n";
			// cmd->function = R_ImageDump_f;
			// cmd->next = nullptr;

			// // Traverse the list to find the last element
			// cmd_function_s *current = cmd_functions;
			// while (current->next != nullptr)
			// 	current = current->next;

			// current->next = cmd;
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
