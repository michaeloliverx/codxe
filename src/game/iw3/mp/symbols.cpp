#include "pch.h"
#include "symbols.h"

namespace iw3
{
namespace mp
{
namespace
{
static const int keycatch_console = 0x1;

void Con_ResetConsoleInputField()
{
    memset(g_consoleField->buffer, 0, sizeof(g_consoleField->buffer));
    g_consoleField->cursor = 0;
    g_consoleField->scroll = 0;
    g_consoleField->drawWidth = 256;
    g_consoleField->widthInPixels = *g_console_field_width;
    g_consoleField->charHeight = *g_console_char_height;
    g_consoleField->fixedSize = 1;
}
} // namespace

void Con_ToggleConsole()
{
    Con_ResetConsoleInputField();
    Con_CancelAutoComplete();

    con->outputVisible = false;
    clientUIActives[0].keyCatchers ^= keycatch_console;
}

void Con_ToggleConsoleOutput()
{
    con->outputVisible = !con->outputVisible;
}

void Con_Bottom()
{
    con->displayLineOffset = con->consoleWindow.activeLineCount;
}

void Con_Top()
{
    if (con->consoleWindow.activeLineCount < con->visibleLineCount)
    {
        con->displayLineOffset = con->consoleWindow.activeLineCount;
    }
    else
    {
        con->displayLineOffset = con->visibleLineCount;
    }
}

void Con_PageDown()
{
    if (con->displayLineOffset + 2 < con->consoleWindow.activeLineCount)
    {
        con->displayLineOffset += 2;
    }
    else
    {
        Con_Bottom();
    }
}

void Con_PageUp()
{
    con->displayLineOffset -= 2;

    if (con->displayLineOffset < con->visibleLineCount)
    {
        Con_Top();
    }
}

void Console_SubmitInput(int localClientNum)
{
    char command_buffer[sizeof(g_consoleField->buffer)];
    strncpy(command_buffer, g_consoleField->buffer, sizeof(command_buffer) - 1);
    command_buffer[sizeof(command_buffer) - 1] = 0;

    const char *command = command_buffer;

    Com_Printf(CON_CHANNEL_DONT_FILTER, "]%s\n", command);

    if (*command == '\\' || *command == '/')
    {
        ++command;
    }

    if (*command)
    {
        Cbuf_AddText(localClientNum, command);
        Cbuf_AddText(localClientNum, "\n");
    }

    Con_ResetConsoleInputField();
}

bool CL_IsConsoleKey(int key)
{
    return key == '`' || key == '^' || key == K_KP_PGUP || key == '~';
}

} // namespace mp
} // namespace iw3
