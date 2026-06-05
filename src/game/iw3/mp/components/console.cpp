#include "pch.h"
#include "console.h"
#include "command.h"

namespace iw3
{
namespace mp
{
namespace
{
static const DWORD keyboard_user_index = 0;
static const DWORD keyboard_flags = XINPUT_FLAG_KEYBOARD;
static const int max_keystrokes_per_frame = 16;
static const int keycatch_console = 0x1;
static connstate_t previous_connection_state = CA_DISCONNECTED;
static bool shift_down = false;
static bool ctrl_down = false;
Detour CL_Input_Detour;

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

bool is_printable_ascii(WCHAR ch)
{
    return ch >= 32 && ch <= 126;
}

void update_modifier_state(int key, bool down)
{
    if (key == K_SHIFT)
    {
        shift_down = down;
    }
    else if (key == K_CTRL)
    {
        ctrl_down = down;
    }
}

bool is_shift_down(const XINPUT_KEYSTROKE &keystroke)
{
    return shift_down || (keystroke.Flags & XINPUT_KEYSTROKE_SHIFT) != 0;
}

bool is_ctrl_down()
{
    return ctrl_down;
}

bool is_large_console_toggle(const XINPUT_KEYSTROKE &keystroke)
{
    return is_shift_down(keystroke);
}

bool is_console_open()
{
    return (clientUIActives[0].keyCatchers & keycatch_console) != 0;
}

bool should_process_key_down(const XINPUT_KEYSTROKE &keystroke)
{
    return (keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN) != 0 || is_repeat(keystroke);
}

void send_console_char_event(int key)
{
    CL_CharEvent(0, key);
}

int console_input_length()
{
    return static_cast<int>(strlen(g_consoleField->buffer));
}

bool is_console_input_alnum(int index)
{
    return isalnum(static_cast<unsigned char>(g_consoleField->buffer[index])) != 0;
}

void adjust_console_input_scroll()
{
    Field_AdjustScroll(&scrPlaceFull, g_consoleField);
}

void console_input_home()
{
    g_consoleField->cursor = 0;
    g_consoleField->scroll = 0;
    adjust_console_input_scroll();
}

void console_input_end()
{
    g_consoleField->cursor = console_input_length();
    adjust_console_input_scroll();
}

void console_input_left(bool ctrl_down)
{
    if (g_consoleField->cursor > 0)
    {
        --g_consoleField->cursor;
    }

    if (ctrl_down)
    {
        while (g_consoleField->cursor > 0 && is_console_input_alnum(g_consoleField->cursor - 1))
        {
            --g_consoleField->cursor;
        }
    }

    if (g_consoleField->cursor < g_consoleField->scroll)
    {
        g_consoleField->scroll = g_consoleField->cursor;
    }

    adjust_console_input_scroll();
}

void console_input_right(bool ctrl_down)
{
    const int len = console_input_length();

    if (g_consoleField->cursor < len)
    {
        ++g_consoleField->cursor;
    }

    if (ctrl_down)
    {
        while (g_consoleField->cursor < len && is_console_input_alnum(g_consoleField->cursor))
        {
            ++g_consoleField->cursor;
        }

        while (g_consoleField->cursor < len && !is_console_input_alnum(g_consoleField->cursor))
        {
            ++g_consoleField->cursor;
        }
    }

    adjust_console_input_scroll();
}

void console_input_delete()
{
    const int len = console_input_length();

    if (g_consoleField->cursor < len)
    {
        memmove(&g_consoleField->buffer[g_consoleField->cursor], &g_consoleField->buffer[g_consoleField->cursor + 1],
                len - g_consoleField->cursor);
        adjust_console_input_scroll();
    }
}

bool dispatch_console_key(int key, const XINPUT_KEYSTROKE &keystroke)
{
    if (!is_console_open())
    {
        return false;
    }

    if (is_keyup(keystroke))
    {
        return true;
    }

    if (!should_process_key_down(keystroke))
    {
        return true;
    }

    switch (key)
    {
    case K_ENTER:
        Console_SubmitInput(0);
        return true;
    case K_PGUP:
        Con_PageUp();
        return true;
    case K_PGDN:
        Con_PageDown();
        return true;
    case K_HOME:
    case K_KP_HOME:
        if (is_ctrl_down())
        {
            Con_Top();
        }
        else
        {
            console_input_home();
        }
        return true;
    case K_END:
    case K_KP_END:
        if (is_ctrl_down())
        {
            Con_Bottom();
        }
        else
        {
            console_input_end();
        }
        return true;
    case K_LEFTARROW:
    case K_KP_LEFTARROW:
        console_input_left(is_ctrl_down());
        return true;
    case K_RIGHTARROW:
    case K_KP_RIGHTARROW:
        console_input_right(is_ctrl_down());
        return true;
    case K_UPARROW:
    case K_KP_UPARROW:
        Console_HistoryPrev();
        return true;
    case K_DOWNARROW:
    case K_KP_DOWNARROW:
        Console_HistoryNext();
        return true;
    case K_DEL:
    case K_KP_DEL:
        console_input_delete();
        return true;
    case K_ESCAPE:
        console::close();
        return true;
    case K_BACKSPACE:
        send_console_char_event('\b');
        return true;
    default:
        break;
    }

    if (is_printable_ascii(keystroke.Unicode))
    {
        send_console_char_event(static_cast<int>(keystroke.Unicode));
    }

    return true;
}

void toggle_large_console()
{
    if (!is_console_open())
    {
        console::toggle();
    }

    Con_ToggleConsoleOutput();
}

void Cmd_ToggleConsole_f()
{
    console::toggle();
}

void CL_Input_Hook(int localClientNum)
{
    if (localClientNum == 0)
    {
        console::frame();
    }

    CL_Input_Detour.GetOriginal<decltype(CL_Input)>()(localClientNum);
}
} // namespace

console::console()
{
    command::add("toggleconsole", Cmd_ToggleConsole_f);

    CL_Input_Detour = Detour(CL_Input, CL_Input_Hook);
    CL_Input_Detour.Install();
}

console::~console()
{
    CL_Input_Detour.Remove();
}

void console::frame()
{
    const connstate_t connection_state = clientUIActives[0].connectionState;

    if (connection_state == CA_LOADING)
    {
        if (previous_connection_state != CA_LOADING)
        {
            close();
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

        if (result != ERROR_SUCCESS)
        {
            return;
        }

        if (keystroke.Flags == 0)
        {
            return;
        }

        const int key = convert_virtual_key(keystroke.VirtualKey, keystroke.Unicode);
        if (!key)
        {
            continue;
        }

        const bool down = !is_keyup(keystroke);
        const unsigned int time = Sys_Milliseconds();
        update_modifier_state(key, down);

        if (CL_IsConsoleKey(key))
        {
            if (down && (keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN))
            {
                if (is_large_console_toggle(keystroke))
                {
                    toggle_large_console();
                }
                else
                {
                    toggle();
                }
            }

            continue;
        }

        if (dispatch_console_key(key, keystroke))
        {
            continue;
        }

        send_key_event(key, down, time);

        if (down && should_process_key_down(keystroke))
        {
            send_char_event(keystroke.Unicode);
        }
    }
}

void console::toggle()
{
    if (clientUIActives[0].connectionState == CA_LOADING)
    {
        close();
        return;
    }

    Con_ToggleConsole();
}

void console::close()
{
    Con_Close(0);
    con->outputVisible = false;
}

int console::convert_virtual_key(WORD virtual_key, WCHAR unicode)
{
    if (unicode == '#')
    {
        return '^';
    }

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
        return K_F1 + (virtual_key - VK_F1);
    }

    if (virtual_key >= VK_NUMPAD0 && virtual_key <= VK_NUMPAD9)
    {
        static const int keypad_numbers[] = {
            K_KP_INS, K_KP_END,        K_KP_DOWNARROW, K_KP_PGDN,    K_KP_LEFTARROW,
            K_KP_5,   K_KP_RIGHTARROW, K_KP_HOME,      K_KP_UPARROW, K_KP_PGUP,
        };

        return keypad_numbers[virtual_key - VK_NUMPAD0];
    }

    switch (virtual_key)
    {
    case VK_BACK:
        return K_BACKSPACE;
    case VK_TAB:
        return K_TAB;
    case VK_RETURN:
        return K_ENTER;
    case VK_ESCAPE:
        return K_ESCAPE;
    case VK_SPACE:
        return K_SPACE;
    case VK_PRIOR:
        return K_PGUP;
    case VK_NEXT:
        return K_PGDN;
    case VK_END:
        return K_END;
    case VK_HOME:
        return K_HOME;
    case VK_LEFT:
        return K_LEFTARROW;
    case VK_UP:
        return K_UPARROW;
    case VK_RIGHT:
        return K_RIGHTARROW;
    case VK_DOWN:
        return K_DOWNARROW;
    case VK_INSERT:
        return K_INS;
    case VK_DELETE:
        return K_DEL;
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT:
        return K_SHIFT;
    case VK_CONTROL:
    case VK_LCONTROL:
    case VK_RCONTROL:
        return K_CTRL;
    case VK_MENU:
    case VK_LMENU:
    case VK_RMENU:
        return K_ALT;
    case VK_CAPITAL:
        return K_CAPSLOCK;
    case VK_PAUSE:
        return K_PAUSE;
    case VK_NUMLOCK:
        return K_KP_NUMLOCK;
    case VK_MULTIPLY:
        return K_KP_STAR;
    case VK_ADD:
        return K_KP_PLUS;
    case VK_SUBTRACT:
        return K_KP_MINUS;
    case VK_DECIMAL:
        return K_KP_DEL;
    case VK_DIVIDE:
        return K_KP_SLASH;
    case VK_OEM_3:
        return '`';
#ifdef VK_OEM_8
    case VK_OEM_8:
        return '^';
#endif
    default:
        return 0;
    }
}

void console::send_key_event(int key, bool down, unsigned int time)
{
    CL_GamepadButtonEvent(0, keyboard_user_index, key, down ? 1 : 0, time);
}

void console::send_char_event(WCHAR unicode)
{
    if (!is_printable_ascii(unicode))
    {
        return;
    }

    if (unicode == '`' || unicode == '~')
    {
        return;
    }

    CL_CharEvent(0, static_cast<int>(unicode));
}

} // namespace mp
} // namespace iw3
