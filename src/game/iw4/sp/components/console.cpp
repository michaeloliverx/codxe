#include "pch.h"
#include "console.h"
#include "game/iw4/common/console_keys.h"

namespace iw4
{
namespace sp
{
namespace
{
using namespace console_keys;

const int local_client_num = 0;
const int max_keystrokes_per_frame = 16;
int pressed_keys[256] = {};
bool shift_down = false;
bool command_registered = false;
cmd_function_s toggle_console_command = {};
Detour Com_Frame_Detour;

bool can_use_console()
{
    return *clientConnectionState == CA_DISCONNECTED || *clientConnectionState == CA_ACTIVE;
}

void toggle_console()
{
    if (can_use_console())
        Con_ToggleConsole();
}

void release_keys()
{
    const unsigned int time = Sys_Milliseconds();
    for (unsigned int i = 0; i < ARRAYSIZE(pressed_keys); ++i)
    {
        if (pressed_keys[i])
        {
            CL_KeyEvent(local_client_num, pressed_keys[i], 0, time);
            pressed_keys[i] = K_NONE;
        }
    }
    shift_down = false;
}

void dispatch_keystroke(const XINPUT_KEYSTROKE &keystroke)
{
    const WORD virtual_key = keystroke.VirtualKey;
    if (virtual_key >= ARRAYSIZE(pressed_keys))
        return;

    const bool down = (keystroke.Flags & (XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_REPEAT)) != 0 &&
                      (keystroke.Flags & XINPUT_KEYSTROKE_KEYUP) == 0;
    int key = convert_virtual_key(virtual_key, keystroke.Unicode);
    if (!key && virtual_key >= 'A' && virtual_key <= 'Z')
        key = virtual_key - 'A' + 'a';

    if (key == K_SHIFT)
        shift_down = down;

    if (key == '`' || key == '~')
    {
        if (down && (keystroke.Flags & XINPUT_KEYSTROKE_REPEAT) == 0)
        {
            if (shift_down || (keystroke.Flags & XINPUT_KEYSTROKE_SHIFT) || key == '~')
            {
                if (!Con_IsActive(local_client_num))
                    Con_ToggleConsole();
                Con_ToggleConsoleOutput();
            }
            else
            {
                toggle_console();
            }
        }
        return;
    }

    // Keep the key from keydown so releases work even when XInput omits Unicode on keyup.
    int &pressed_key = pressed_keys[virtual_key];
    if (!down)
    {
        if (pressed_key)
        {
            CL_KeyEvent(local_client_num, pressed_key, 0, Sys_Milliseconds());
            pressed_key = K_NONE;
        }
        return;
    }

    if (!key || !Con_IsActive(local_client_num))
        return;

    if (!pressed_key)
        pressed_key = key;
    CL_KeyEvent(local_client_num, pressed_key, 1, Sys_Milliseconds());
    if (key == K_BACKSPACE)
        CL_CharEvent(local_client_num, '\b');
    else if (is_printable_ascii(keystroke.Unicode))
        CL_CharEvent(local_client_num, keystroke.Unicode);
}

void poll_keyboard()
{
    const bool console_available = can_use_console();
    if (!console_available && Con_IsActive(local_client_num))
    {
        release_keys();
        Con_ToggleConsole();
    }

    for (int i = 0; i < max_keystrokes_per_frame; ++i)
    {
        XINPUT_KEYSTROKE keystroke = {};
        const DWORD result = XInputGetKeystroke(0, XINPUT_FLAG_KEYBOARD, &keystroke);
        if (result == ERROR_DEVICE_NOT_CONNECTED)
            release_keys();
        if (result != ERROR_SUCCESS || !keystroke.Flags)
            break;
        if (console_available)
            dispatch_keystroke(keystroke);
    }
}

int Com_Frame_Hook()
{
    if (!command_registered)
    {
        Cmd_AddCommandInternal("toggleconsole", toggle_console, &toggle_console_command);
        command_registered = true;
        DbgPrint("[codxe][iw4_sp] Console input initialized\n");
    }

    poll_keyboard();
    return Com_Frame_Detour.GetOriginal<decltype(Com_Frame)>()();
}
} // namespace

console::console()
{
    Com_Frame_Detour = Detour(Com_Frame, Com_Frame_Hook);
    Com_Frame_Detour.Install();
}
} // namespace sp
} // namespace iw4
