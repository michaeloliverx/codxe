#pragma once

namespace iw4
{
namespace console_keys
{
enum keyNum_t : __int32
{
    K_NONE = 0x0,
    K_TAB = 0x9,
    K_ENTER = 0xD,
    K_ESCAPE = 0x1B,
    K_SPACE = 0x20,
    K_BACKSPACE = 0x7F,
    K_CAPSLOCK = 0x97,
    K_PAUSE = 0x99,
    K_UPARROW = 0x9A,
    K_DOWNARROW = 0x9B,
    K_LEFTARROW = 0x9C,
    K_RIGHTARROW = 0x9D,
    K_ALT = 0x9E,
    K_CTRL = 0x9F,
    K_SHIFT = 0xA0,
    K_INS = 0xA1,
    K_DEL = 0xA2,
    K_PGDN = 0xA3,
    K_PGUP = 0xA4,
    K_HOME = 0xA5,
    K_END = 0xA6,
    K_F1 = 0xA7,
    K_F2 = 0xA8,
    K_F3 = 0xA9,
    K_F4 = 0xAA,
    K_F5 = 0xAB,
    K_F6 = 0xAC,
    K_F7 = 0xAD,
    K_F8 = 0xAE,
    K_F9 = 0xAF,
    K_F10 = 0xB0,
    K_F11 = 0xB1,
    K_F12 = 0xB2,
    K_KP_HOME = 0xB6,
    K_KP_UPARROW = 0xB7,
    K_KP_PGUP = 0xB8,
    K_KP_LEFTARROW = 0xB9,
    K_KP_5 = 0xBA,
    K_KP_RIGHTARROW = 0xBB,
    K_KP_END = 0xBC,
    K_KP_DOWNARROW = 0xBD,
    K_KP_PGDN = 0xBE,
    K_KP_ENTER = 0xBF,
    K_KP_INS = 0xC0,
    K_KP_DEL = 0xC1,
    K_KP_SLASH = 0xC2,
    K_KP_MINUS = 0xC3,
    K_KP_PLUS = 0xC4,
    K_KP_NUMLOCK = 0xC5,
    K_KP_STAR = 0xC6,
    K_KP_EQUALS = 0xC7,
};

inline bool is_printable_ascii(WCHAR ch)
{
    return ch >= 32 && ch <= 126;
}

inline int convert_virtual_key(WORD virtual_key, WCHAR unicode)
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
    default:
        return K_NONE;
    }
}
} // namespace console_keys
} // namespace iw4
