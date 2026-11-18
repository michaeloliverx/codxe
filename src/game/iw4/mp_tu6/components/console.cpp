#include "pch.h"
#include "console.h"
#include "game/iw4/common/console_keys.h"

namespace
{
using namespace iw4::console_keys;

static const DWORD keyboard_user_index = 0;
static const DWORD keyboard_flags = XINPUT_FLAG_KEYBOARD;
static const int local_client_num = 0;
static const int max_keystrokes_per_frame = 16;

iw4::mp_tu6::connstate_t previous_connection_state = iw4::mp_tu6::CA_DISCONNECTED;
bool shift_down = false;
iw4::mp_tu6::cmd_function_s Cmd_ToggleConsole_VAR;

bool is_keyup(const XINPUT_KEYSTROKE &keystroke)
{
    return (keystroke.Flags & XINPUT_KEYSTROKE_KEYUP) != 0;
}

bool is_repeat(const XINPUT_KEYSTROKE &keystroke)
{
#ifdef XINPUT_KEYSTROKE_REPEAT
    return (keystroke.Flags & XINPUT_KEYSTROKE_REPEAT) != 0;
#else
    return (keystroke.Flags & 4) != 0;
#endif
}

bool has_shift_flag(const XINPUT_KEYSTROKE &keystroke)
{
#ifdef XINPUT_KEYSTROKE_SHIFT
    return (keystroke.Flags & XINPUT_KEYSTROKE_SHIFT) != 0;
#else
    return false;
#endif
}

bool should_process_key_down(const XINPUT_KEYSTROKE &keystroke)
{
    return (keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN) != 0 || is_repeat(keystroke);
}

void update_modifier_state(int key, bool down)
{
    if (key == K_SHIFT)
    {
        shift_down = down;
    }
}

bool is_shift_down(const XINPUT_KEYSTROKE &keystroke)
{
    return shift_down || has_shift_flag(keystroke);
}

bool is_console_key(int key)
{
    return key == '`' || key == '~';
}

void close_console()
{
    iw4::mp_tu6::Con_Close(local_client_num);
    iw4::mp_tu6::con->outputVisible = false;
}

void toggle_console()
{
    if (iw4::mp_tu6::clientUIActives[local_client_num].connectionState == iw4::mp_tu6::CA_LOADING)
    {
        close_console();
        return;
    }

    iw4::mp_tu6::Con_ToggleConsole();
}

void toggle_large_console()
{
    if (!iw4::mp_tu6::Con_IsActive(local_client_num))
    {
        toggle_console();
    }

    iw4::mp_tu6::Con_ToggleConsoleOutput();
}

void Cmd_ToggleConsole_f()
{
    toggle_console();
}

void register_commands()
{
    iw4::mp_tu6::Cmd_AddCommandInternal("toggleconsole", Cmd_ToggleConsole_f, &Cmd_ToggleConsole_VAR);
}

void send_char_event(int key, const XINPUT_KEYSTROKE &keystroke)
{
    if (key == K_BACKSPACE)
    {
        iw4::mp_tu6::CL_CharEvent(local_client_num, '\b');
        return;
    }

    if (!is_printable_ascii(keystroke.Unicode))
    {
        return;
    }

    if (keystroke.Unicode == '`' || keystroke.Unicode == '~')
    {
        return;
    }

    iw4::mp_tu6::CL_CharEvent(local_client_num, static_cast<int>(keystroke.Unicode));
}

void dispatch_keystroke(const XINPUT_KEYSTROKE &keystroke)
{
    const int key = convert_virtual_key(keystroke.VirtualKey, keystroke.Unicode);
    if (!key)
    {
        return;
    }

    const bool down = !is_keyup(keystroke);
    update_modifier_state(key, down);

    if (is_console_key(key))
    {
        if (down && (keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN))
        {
            if (is_shift_down(keystroke) || key == '~')
            {
                toggle_large_console();
            }
            else
            {
                toggle_console();
            }
        }

        return;
    }

    iw4::mp_tu6::CL_KeyEvent(local_client_num, key, down ? 1 : 0, iw4::mp_tu6::Sys_Milliseconds());

    if (down && should_process_key_down(keystroke))
    {
        send_char_event(key, keystroke);
    }
}
} // namespace

Detour console::SCR_DrawScreenField_Detour;

void console::OnCmdInit()
{
    register_commands();
}

console::console()
{
    SCR_DrawScreenField_Detour = Detour(iw4::mp_tu6::SCR_DrawScreenField, SCR_DrawScreenField_Hook);
    SCR_DrawScreenField_Detour.Install();
}

void console::Frame()
{
    const iw4::mp_tu6::connstate_t connection_state = iw4::mp_tu6::clientUIActives[local_client_num].connectionState;

    if (connection_state == iw4::mp_tu6::CA_LOADING)
    {
        if (previous_connection_state != iw4::mp_tu6::CA_LOADING)
        {
            close_console();
        }

        previous_connection_state = connection_state;
        return;
    }

    previous_connection_state = connection_state;

    for (int i = 0; i < max_keystrokes_per_frame; ++i)
    {
        XINPUT_KEYSTROKE keystroke = {};
        const DWORD result = XInputGetKeystroke(keyboard_user_index, keyboard_flags, &keystroke);

        if (result == ERROR_EMPTY || result == ERROR_DEVICE_NOT_CONNECTED)
        {
            return;
        }

        if (result != ERROR_SUCCESS || keystroke.Flags == 0)
        {
            return;
        }

        dispatch_keystroke(keystroke);
    }
}

void console::SCR_DrawScreenField_Hook(int localClientNum, int refreshedUI)
{
    Frame();
    SCR_DrawScreenField_Detour.GetOriginal<decltype(iw4::mp_tu6::SCR_DrawScreenField)>()(localClientNum, refreshedUI);
}
