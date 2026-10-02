#include "pch.h"
#include "console.h"

namespace iw5
{
namespace mp
{
namespace
{
const DWORD keyboard_user_index = 0;
const DWORD keyboard_flags = XINPUT_FLAG_KEYBOARD;
const int max_keystrokes_per_frame = 16;

const int max_lines = 128;
const int max_line_length = 160;
const int visible_lines = 16;
const int max_input_length = 200;

// Layout in 640x480 virtual space (HORIZONTAL_ALIGN_LEFT / VERTICAL_ALIGN_TOP)
const int horz_align_left = 1;
const int vert_align_top = 1;
const float text_x = 12.0f;
const float text_y = 24.0f;
const float line_height = 11.0f;
const float text_scale = 0.22f;

const float color_output[4] = {1.0f, 1.0f, 1.0f, 0.9f};
const float color_input[4] = {1.0f, 1.0f, 0.4f, 1.0f};
const float color_hint[4] = {0.7f, 0.7f, 0.7f, 0.8f};

CRITICAL_SECTION lines_lock;
char lines[max_lines][max_line_length];
int line_next = 0;
int line_count = 0;
int scroll_offset = 0;

bool console_open = false;
char input_line[max_input_length + 1];
int input_length = 0;

void AddLine(const char *text, int length)
{
    if (length >= max_line_length)
        length = max_line_length - 1;

    EnterCriticalSection(&lines_lock);
    memcpy(lines[line_next], text, length);
    lines[line_next][length] = '\0';
    line_next = (line_next + 1) % max_lines;
    if (line_count < max_lines)
        line_count++;
    LeaveCriticalSection(&lines_lock);
}

// CL_ConsolePrint text can hold several lines and usually ends with '\n'
void AddText(const char *txt)
{
    if (!txt)
        return;

    const char *start = txt;
    for (const char *p = txt;; ++p)
    {
        if (*p == '\n' || *p == '\0')
        {
            if (p > start)
                AddLine(start, static_cast<int>(p - start));

            if (*p == '\0')
                break;

            start = p + 1;
        }
    }
}

void ExecuteInput()
{
    if (input_length == 0)
        return;

    char echo[max_input_length + 3];
    _snprintf(echo, sizeof(echo), "]%s", input_line);
    echo[sizeof(echo) - 1] = '\0';
    AddText(echo);

    char command[max_input_length + 2];
    _snprintf(command, sizeof(command), "%s\n", input_line);
    command[sizeof(command) - 1] = '\0';
    Cbuf_AddText(0, command);

    input_length = 0;
    input_line[0] = '\0';
    scroll_offset = 0;
}

bool IsConsoleKey(const XINPUT_KEYSTROKE &keystroke)
{
    return keystroke.VirtualKey == VK_OEM_3 || keystroke.Unicode == '`' || keystroke.Unicode == '~';
}

bool IsRepeat(const XINPUT_KEYSTROKE &keystroke)
{
#ifdef XINPUT_KEYSTROKE_REPEAT
    return (keystroke.Flags & XINPUT_KEYSTROKE_REPEAT) != 0;
#else
    return (keystroke.Flags & 4) != 0;
#endif
}

void HandleKeystroke(const XINPUT_KEYSTROKE &keystroke)
{
    if ((keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN) == 0 && !IsRepeat(keystroke))
        return;

    if (IsConsoleKey(keystroke))
    {
        if (keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN)
            console_open = !console_open;
        return;
    }

    if (!console_open)
        return;

    switch (keystroke.VirtualKey)
    {
    case VK_RETURN:
        ExecuteInput();
        return;
    case VK_BACK:
        if (input_length > 0)
            input_line[--input_length] = '\0';
        return;
    case VK_ESCAPE:
        console_open = false;
        return;
    case VK_PRIOR:
        scroll_offset += visible_lines / 2;
        if (scroll_offset > line_count - visible_lines)
            scroll_offset = line_count > visible_lines ? line_count - visible_lines : 0;
        return;
    case VK_NEXT:
        scroll_offset -= visible_lines / 2;
        if (scroll_offset < 0)
            scroll_offset = 0;
        return;
    default:
        break;
    }

    const WCHAR ch = keystroke.Unicode;
    if (ch >= 32 && ch <= 126 && input_length < max_input_length)
    {
        input_line[input_length++] = static_cast<char>(ch);
        input_line[input_length] = '\0';
    }
}

void PollKeyboard()
{
    for (int i = 0; i < max_keystrokes_per_frame; ++i)
    {
        XINPUT_KEYSTROKE keystroke = {};
        const DWORD result = XInputGetKeystroke(keyboard_user_index, keyboard_flags, &keystroke);

        if (result != ERROR_SUCCESS || keystroke.Flags == 0)
            return;

        HandleKeystroke(keystroke);
    }
}

void Draw(LocalClientNum_t localClientNum)
{
    const ScreenPlacement *placement = ScrPlace_GetActivePlacement(localClientNum);
    Font_s *font = sharedUiInfo->assets.consoleFont;
    if (!placement || !font)
        return;

    EnterCriticalSection(&lines_lock);
    const int shown = line_count < visible_lines ? line_count : visible_lines;
    for (int i = 0; i < shown; ++i)
    {
        // oldest visible line first
        const int age = scroll_offset + shown - 1 - i;
        const int index = (line_next - 1 - age + max_lines * 2) % max_lines;
        UI_DrawText(placement, lines[index], max_line_length, font, text_x, text_y + line_height * i, horz_align_left,
                    vert_align_top, text_scale, color_output, 0);
    }
    LeaveCriticalSection(&lines_lock);

    char prompt[max_input_length + 4];
    const bool cursor_on = (GetTickCount() / 500) % 2 == 0;
    _snprintf(prompt, sizeof(prompt), "]%s%s", input_line, cursor_on ? "_" : "");
    prompt[sizeof(prompt) - 1] = '\0';
    UI_DrawText(placement, prompt, max_input_length + 2, font, text_x, text_y + line_height * (visible_lines + 1),
                horz_align_left, vert_align_top, text_scale, color_input, 0);

    if (scroll_offset > 0)
        UI_DrawText(placement, "^3-- scrolled (PgDn) --", 32, font, text_x,
                    text_y + line_height * (visible_lines + 2), horz_align_left, vert_align_top, text_scale,
                    color_hint, 0);
}
} // namespace

Detour Console::UI_Refresh_Detour;
Detour Console::CL_ConsolePrint_Detour;

void Console::UI_Refresh_Hook(LocalClientNum_t localClientNum)
{
    UI_Refresh_Detour.GetOriginal<UI_Refresh_t>()(localClientNum);

    // UI refresh runs once per local client; the keyboard belongs to the first one
    if (localClientNum != 0)
        return;

    PollKeyboard();

    if (console_open)
        Draw(localClientNum);
}

void Console::CL_ConsolePrint_Hook(LocalClientNum_t localClientNum, int channel, const char *txt,
                                   unsigned int duration, unsigned int pixelWidth, int flags)
{
#ifndef NDEBUG
    DbgPrint("[codxe][IW5][CL_ConsolePrint] %s\n", txt);
#endif
    AddText(txt);
    CL_ConsolePrint_Detour.GetOriginal<CL_ConsolePrint_t>()(localClientNum, channel, txt, duration, pixelWidth, flags);
}

Console::Console()
{
    InitializeCriticalSection(&lines_lock);
    input_line[0] = '\0';

    CL_ConsolePrint_Detour = Detour(CL_ConsolePrint, CL_ConsolePrint_Hook);
    CL_ConsolePrint_Detour.Install();

    UI_Refresh_Detour = Detour(UI_Refresh, UI_Refresh_Hook);
    UI_Refresh_Detour.Install();
}

Console::~Console()
{
    UI_Refresh_Detour.Remove();
    CL_ConsolePrint_Detour.Remove();
    DeleteCriticalSection(&lines_lock);
}
} // namespace mp
} // namespace iw5
