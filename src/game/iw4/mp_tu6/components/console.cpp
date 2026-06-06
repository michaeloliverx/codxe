#include "pch.h"
#include "console.h"
#include "events.h"

namespace
{
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

bool is_printable_ascii(WCHAR ch)
{
    return ch >= 32 && ch <= 126;
}

bool should_process_key_down(const XINPUT_KEYSTROKE &keystroke)
{
    return (keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN) != 0 || is_repeat(keystroke);
}

void update_modifier_state(int key, bool down)
{
    if (key == iw4::mp_tu6::K_SHIFT)
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

int convert_virtual_key(WORD virtual_key, WCHAR unicode)
{
    if (virtual_key == VK_OEM_3)
    {
        return unicode == '~' ? '~' : '`';
    }

#ifdef VK_OEM_8
    if (virtual_key == VK_OEM_8)
    {
        return unicode == '~' ? '~' : '`';
    }
#endif

    if (is_printable_ascii(unicode))
    {
        if (unicode >= 'A' && unicode <= 'Z')
        {
            return static_cast<int>(unicode - 'A' + 'a');
        }

        return static_cast<int>(unicode);
    }

    if (virtual_key >= VK_F1 && virtual_key <= VK_F12)
    {
        return iw4::mp_tu6::K_F1 + (virtual_key - VK_F1);
    }

    if (virtual_key >= VK_NUMPAD0 && virtual_key <= VK_NUMPAD9)
    {
        static const int keypad_numbers[] = {
            iw4::mp_tu6::K_KP_INS,       iw4::mp_tu6::K_KP_END,  iw4::mp_tu6::K_KP_DOWNARROW,  iw4::mp_tu6::K_KP_PGDN,
            iw4::mp_tu6::K_KP_LEFTARROW, iw4::mp_tu6::K_KP_5,    iw4::mp_tu6::K_KP_RIGHTARROW, iw4::mp_tu6::K_KP_HOME,
            iw4::mp_tu6::K_KP_UPARROW,   iw4::mp_tu6::K_KP_PGUP,
        };

        return keypad_numbers[virtual_key - VK_NUMPAD0];
    }

    switch (virtual_key)
    {
    case VK_BACK:
        return iw4::mp_tu6::K_BACKSPACE;
    case VK_TAB:
        return iw4::mp_tu6::K_TAB;
    case VK_RETURN:
        return iw4::mp_tu6::K_ENTER;
    case VK_ESCAPE:
        return iw4::mp_tu6::K_ESCAPE;
    case VK_SPACE:
        return iw4::mp_tu6::K_SPACE;
    case VK_PRIOR:
        return iw4::mp_tu6::K_PGUP;
    case VK_NEXT:
        return iw4::mp_tu6::K_PGDN;
    case VK_END:
        return iw4::mp_tu6::K_END;
    case VK_HOME:
        return iw4::mp_tu6::K_HOME;
    case VK_LEFT:
        return iw4::mp_tu6::K_LEFTARROW;
    case VK_UP:
        return iw4::mp_tu6::K_UPARROW;
    case VK_RIGHT:
        return iw4::mp_tu6::K_RIGHTARROW;
    case VK_DOWN:
        return iw4::mp_tu6::K_DOWNARROW;
    case VK_INSERT:
        return iw4::mp_tu6::K_INS;
    case VK_DELETE:
        return iw4::mp_tu6::K_DEL;
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT:
        return iw4::mp_tu6::K_SHIFT;
    case VK_CONTROL:
    case VK_LCONTROL:
    case VK_RCONTROL:
        return iw4::mp_tu6::K_CTRL;
    case VK_MENU:
    case VK_LMENU:
    case VK_RMENU:
        return iw4::mp_tu6::K_ALT;
    case VK_CAPITAL:
        return iw4::mp_tu6::K_CAPSLOCK;
    case VK_PAUSE:
        return iw4::mp_tu6::K_PAUSE;
    case VK_NUMLOCK:
        return iw4::mp_tu6::K_KP_NUMLOCK;
    case VK_MULTIPLY:
        return iw4::mp_tu6::K_KP_STAR;
    case VK_ADD:
        return iw4::mp_tu6::K_KP_PLUS;
    case VK_SUBTRACT:
        return iw4::mp_tu6::K_KP_MINUS;
    case VK_DECIMAL:
        return iw4::mp_tu6::K_KP_DEL;
    case VK_DIVIDE:
        return iw4::mp_tu6::K_KP_SLASH;
    default:
        return iw4::mp_tu6::K_NONE;
    }
}

void send_char_event(int key, const XINPUT_KEYSTROKE &keystroke)
{
    if (key == iw4::mp_tu6::K_BACKSPACE)
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

console::console()
{
    Events::OnCmdInit(register_commands);

    SCR_DrawScreenField_Detour = Detour(iw4::mp_tu6::SCR_DrawScreenField, SCR_DrawScreenField_Hook);
    SCR_DrawScreenField_Detour.Install();
}

console::~console()
{
    SCR_DrawScreenField_Detour.Remove();
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
