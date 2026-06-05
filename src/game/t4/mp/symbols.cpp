#include "pch.h"
#include "symbols.h"

namespace t4
{
namespace mp
{
namespace
{
static const int keycatch_console = 0x1;
static const int console_history_count = 32;
static int historyLine = 0;
static int nextHistoryLine = 0;
static field_t historyEditLines[console_history_count];

void Con_ResetConsoleInputField()
{
    memset(g_consoleField->buffer, 0, sizeof(g_consoleField->buffer));
    g_consoleField->cursor = 0;
    g_consoleField->scroll = 0;
    g_consoleField->drawWidth = 256;

    if (!g_consoleField->widthInPixels)
    {
        g_consoleField->widthInPixels = 640;
    }

    if (g_consoleField->charHeight == 0.0f)
    {
        g_consoleField->charHeight = 16.0f;
    }

    g_consoleField->fixedSize = 1;
}

void Con_CancelAutoComplete()
{
    *reinterpret_cast<char *>(0x826B86A0) = 0;
    *reinterpret_cast<int *>(0x826B86E0) = -1;
}

void Con_CopyHistoryLine(int line)
{
    memcpy(g_consoleField, &historyEditLines[line % console_history_count], sizeof(*g_consoleField));
    Field_AdjustScroll(&scrPlaceFull, g_consoleField);
}

void Console_StoreInputHistory()
{
    if (!g_consoleField->buffer[0])
    {
        return;
    }

    memcpy(&historyEditLines[nextHistoryLine % console_history_count], g_consoleField, sizeof(*g_consoleField));
    historyLine = ++nextHistoryLine;
}
} // namespace

bool SV_IsClientBot(unsigned int clientNum)
{
    assert(clientNum < (*sv_maxclients)->current.unsignedInt);

    return svsHeader->clients[clientNum].header.netchan.remoteAddress.type == NA_BOT;
}

void Con_ToggleConsole()
{
    if (!con->initialized)
    {
        Con_OneTimeInit();
    }

    Con_ResetConsoleInputField();
    Con_CancelAutoComplete();

    con->outputVisible = false;

    if (Key_IsCatcherActive(0, keycatch_console))
    {
        Key_RemoveCatcher(0, ~keycatch_console);
    }
    else
    {
        Key_AddCatcher(0, keycatch_console);
    }
}

void Con_ToggleConsoleOutput()
{
    con->outputVisible = !con->outputVisible;
}

void Con_Close(int localClientNum)
{
    Key_RemoveCatcher(localClientNum, ~keycatch_console);
    Con_CancelAutoComplete();
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

    Com_Printf(0, "]%s\n", command);

    if (*command == '\\' || *command == '/')
    {
        ++command;
    }

    if (*command)
    {
        Cbuf_AddText(localClientNum, command);
        Cbuf_AddText(localClientNum, "\n");
    }

    Console_StoreInputHistory();
    Con_ResetConsoleInputField();
}

void Console_HistoryNext()
{
    if (historyLine == nextHistoryLine)
    {
        return;
    }

    ++historyLine;

    if (historyLine == nextHistoryLine)
    {
        Con_ResetConsoleInputField();
        return;
    }

    Con_CopyHistoryLine(historyLine);
}

void Console_HistoryPrev()
{
    if (!nextHistoryLine)
    {
        return;
    }

    if (nextHistoryLine - historyLine < console_history_count && historyLine > 0)
    {
        --historyLine;
    }

    Con_CopyHistoryLine(historyLine);
}

bool CL_IsConsoleKey(int key)
{
    return key == '`' || key == '^' || key == '~';
}

} // namespace mp
} // namespace t4
