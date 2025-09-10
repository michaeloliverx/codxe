#include "console.h"
#include "common.h"

typedef void (*R_AddCmdDrawStretchPic_t)(float x, float y, float w, float h, float s0, float t0, float s1, float t1,
                                         const float *color, iw4::mp::Material *material);
static auto R_AddCmdDrawStretchPic = reinterpret_cast<R_AddCmdDrawStretchPic_t>(0x823C6DB0);

typedef iw4::mp::Material *(*Material_RegisterHandle_t)(const char *name);
Material_RegisterHandle_t Material_RegisterHandle = reinterpret_cast<Material_RegisterHandle_t>(0x823C2FF8);

typedef void (*R_AddCmdDrawText_t)(const char *text, int maxChars, iw4::mp::Font_s *font, float x, float y,
                                   float xScale, float yScale, float rotation, const float *color, int style);
R_AddCmdDrawText_t R_AddCmdDrawText = reinterpret_cast<R_AddCmdDrawText_t>(0x823C7690);

typedef int (*R_TextWidth_t)(const char *text, int maxChars, iw4::mp::Font_s *font);
R_TextWidth_t R_TextWidth = reinterpret_cast<R_TextWidth_t>(0x823C28F8);

typedef int (*R_TextHeight_t)(iw4::mp::Font_s *font);
R_TextHeight_t R_TextHeight = reinterpret_cast<R_TextHeight_t>(0x823C28F8);

typedef unsigned int (*Sys_Milliseconds_t)();
Sys_Milliseconds_t Sys_Milliseconds = reinterpret_cast<Sys_Milliseconds_t>(0x823401C8);

typedef void (*Com_PrintMessage_t)(int channel, const char *msg, int error);
Com_PrintMessage_t Com_PrintMessage = reinterpret_cast<Com_PrintMessage_t>(0x8227F370);

typedef void (*Com_Printf_t)(int channel, const char *fmt, ...);
Com_Printf_t Com_Printf = reinterpret_cast<Com_Printf_t>(0x8227F448);

typedef void (*CL_ConsolePrint_t)(int localClientNum, int channel, const char *txt, unsigned int duration,
                                  unsigned int pixelWidth, int flags);
CL_ConsolePrint_t CL_ConsolePrint = reinterpret_cast<CL_ConsolePrint_t>(0x821754B8);

// Screen dimensions
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// Console dimensions
#define CONSOLE_WIDTH 1270.0f
#define CONSOLE_HEIGHT 400.0f
#define CONSOLE_X 5.0f
#define CONSOLE_Y 5.0f

// Text settings
#define TEXT_MARGIN 10.0f
#define LINE_HEIGHT 20.0f
#define MAX_INPUT_LENGTH 256
#define HISTORY_BUFFER_SIZE 0x8000 // Circular buffer size
#define CURSOR_BLINK_TIME 500      // milliseconds

const Console::Settings settings = {
    {1.0f, 1.0f, 1.0f, 1.0f},  // text_color
    {0.0f, 0.0f, 0.0f, 0.85f}, // background_color
    {0.8f, 0.8f, 0.8f, 0.8f},  // input_text_color
    {1.0f, 1.0f, 0.0f, 1.0f}   // prompt_color
};

// Console state
struct ConsoleState
{
    bool isOpen;
    char inputBuffer[MAX_INPUT_LENGTH];
    int inputCursor;
    char historyBuffer[HISTORY_BUFFER_SIZE]; // Circular buffer for history
    int writePos;                            // Position where next line will be written
    int readPos;                             // Position where we start reading from
    int lineCount;                           // Number of lines in the buffer
    int historyScroll;
    unsigned int lastCursorBlink;
    bool cursorVisible;

    ConsoleState()
        : isOpen(false), inputCursor(0), writePos(0), readPos(0), lineCount(0), historyScroll(0), lastCursorBlink(0),
          cursorVisible(true)
    {
        memset(inputBuffer, 0, sizeof(inputBuffer));
        memset(historyBuffer, 0, sizeof(historyBuffer));

        // Add welcome message
        AddToHistory("Console initialized. Type 'help' for commands.");
    }

    void AddToHistory(const char *text)
    {
        int textLen = strlen(text);
        if (textLen == 0 || textLen >= HISTORY_BUFFER_SIZE - 1)
            return;

        // Add null terminator to the length
        int requiredSpace = textLen + 1;

        // Check if we need to wrap around
        if (writePos + requiredSpace > HISTORY_BUFFER_SIZE)
        {
            // Mark end of buffer and wrap to beginning
            historyBuffer[writePos] = '\0';
            writePos = 0;
        }

        // Check if we're about to overwrite old data
        if (lineCount > 0 && writePos <= readPos && writePos + requiredSpace > readPos)
        {
            // We need to move the read position forward to the next line
            while (readPos < HISTORY_BUFFER_SIZE && historyBuffer[readPos] != '\0')
                readPos++;
            readPos++; // Skip the null terminator

            // If we hit the end, wrap to beginning
            if (readPos >= HISTORY_BUFFER_SIZE)
                readPos = 0;

            // Skip any lines that will be overwritten
            while (readPos < writePos + requiredSpace && readPos < HISTORY_BUFFER_SIZE)
            {
                if (historyBuffer[readPos] == '\0')
                {
                    readPos++;
                    if (lineCount > 0)
                        lineCount--;
                }
                else
                {
                    // Find the end of this line
                    while (readPos < HISTORY_BUFFER_SIZE && historyBuffer[readPos] != '\0')
                        readPos++;
                    readPos++; // Skip the null terminator
                    if (lineCount > 0)
                        lineCount--;
                }
            }

            if (readPos >= HISTORY_BUFFER_SIZE)
                readPos = 0;
        }

        // Copy the text to the buffer
        strcpy(&historyBuffer[writePos], text);
        writePos += requiredSpace;
        lineCount++;

        // Wrap write position if needed
        if (writePos >= HISTORY_BUFFER_SIZE)
            writePos = 0;
    }

    // Get a line from history by index (0 = oldest visible line)
    const char *GetHistoryLine(int index)
    {
        if (index < 0 || index >= lineCount)
            return NULL;

        int currentPos = readPos;
        int currentIndex = 0;

        while (currentIndex < index && currentIndex < lineCount)
        {
            // Skip to next line
            while (currentPos < HISTORY_BUFFER_SIZE && historyBuffer[currentPos] != '\0')
                currentPos++;
            currentPos++; // Skip null terminator

            if (currentPos >= HISTORY_BUFFER_SIZE)
                currentPos = 0;

            currentIndex++;
        }

        if (historyBuffer[currentPos] == '\0')
            return NULL;

        return &historyBuffer[currentPos];
    }
};

static ConsoleState consoleState;

// Simple command processor
void ProcessCommand(const char *command)
{
    // Add command to history with prompt
    char cmdLine[MAX_INPUT_LENGTH];
    _snprintf(cmdLine, sizeof(cmdLine), "> %s", command);
    consoleState.AddToHistory(cmdLine);

    // Parse command (simple space-separated)
    char cmd[64] = {0};
    char args[192] = {0};
    sscanf(command, "%63s %191[^\n]", cmd, args);

    // Process built-in commands
    if (strcmp(cmd, "help") == 0)
    {
        consoleState.AddToHistory("Available commands:");
        consoleState.AddToHistory("  help - Show this help");
        consoleState.AddToHistory("  clear - Clear console");
        consoleState.AddToHistory("  echo <text> - Echo text back");
        consoleState.AddToHistory("  status - Show game status");
        consoleState.AddToHistory("  quit - Close console");
    }
    else if (strcmp(cmd, "clear") == 0)
    {
        // Reset the circular buffer
        consoleState.writePos = 0;
        consoleState.readPos = 0;
        consoleState.lineCount = 0;
        memset(consoleState.historyBuffer, 0, sizeof(consoleState.historyBuffer));
        consoleState.AddToHistory("Console cleared.");
    }
    else if (strcmp(cmd, "echo") == 0)
    {
        if (strlen(args) > 0)
        {
            consoleState.AddToHistory(args);
        }
        else
        {
            consoleState.AddToHistory("Usage: echo <text>");
        }
    }
    else if (strcmp(cmd, "status") == 0)
    {
        consoleState.AddToHistory("Game Status: Running");
        consoleState.AddToHistory("Console Version: 1.0");
    }
    else if (strcmp(cmd, "quit") == 0)
    {
        consoleState.isOpen = false;
    }
    else if (strlen(cmd) > 0)
    {
        char response[MAX_INPUT_LENGTH];
        _snprintf(response, sizeof(response), "Unknown command: %s", cmd);
        consoleState.AddToHistory(response);
    }
}

void Console::RenderConsole()
{
    if (!consoleState.isOpen)
    {
        return;
    }

    // Draw background
    R_AddCmdDrawStretchPic(CONSOLE_X, CONSOLE_Y, CONSOLE_WIDTH, CONSOLE_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f,
                           settings.background_color, iw4::mp::sharedUiInfo->assets.whiteMaterial);

    // Draw border (optional, for better visibility)
    const float borderColor[4] = {0.5f, 0.5f, 0.5f, 1.0f};
    R_AddCmdDrawStretchPic(CONSOLE_X, CONSOLE_Y, CONSOLE_WIDTH, 2.0f, 0.0f, 0.0f, 1.0f, 1.0f, borderColor,
                           iw4::mp::sharedUiInfo->assets.whiteMaterial);
    R_AddCmdDrawStretchPic(CONSOLE_X, CONSOLE_Y + CONSOLE_HEIGHT - 2.0f, CONSOLE_WIDTH, 2.0f, 0.0f, 0.0f, 1.0f, 1.0f,
                           borderColor, iw4::mp::sharedUiInfo->assets.whiteMaterial);

    // The text anchor is at the bottom left of the first line, not top left
    // So we need to add the line height to position the first line correctly
    int fontHeight = R_TextHeight(iw4::mp::sharedUiInfo->assets.consoleFont);
    float scaledFontHeight = fontHeight * 0.8f; // Account for the 0.8f scale we use when drawing
    float yPos = CONSOLE_Y + TEXT_MARGIN + scaledFontHeight;

    // Draw history (from bottom up, leaving space for input)
    int maxVisibleLines = (int)((CONSOLE_HEIGHT - 50.0f) / scaledFontHeight);

    // Calculate the starting line based on scroll position
    int startLine = 0;
    int endLine = consoleState.lineCount;

    if (consoleState.lineCount > maxVisibleLines)
    {
        // Default to showing the bottom (most recent) lines
        startLine = consoleState.lineCount - maxVisibleLines;

        // Apply scroll offset (negative scroll moves view up, showing older messages)
        startLine -= consoleState.historyScroll;

        // Clamp to valid range
        if (startLine < 0)
            startLine = 0;
        if (startLine > consoleState.lineCount - maxVisibleLines)
            startLine = consoleState.lineCount - maxVisibleLines;

        endLine = startLine + maxVisibleLines;
        if (endLine > consoleState.lineCount)
            endLine = consoleState.lineCount;
    }

    for (int i = startLine; i < endLine; i++)
    {
        const char *line = consoleState.GetHistoryLine(i);
        if (line)
        {
            R_AddCmdDrawText(line, MAX_INPUT_LENGTH, iw4::mp::sharedUiInfo->assets.consoleFont, CONSOLE_X + TEXT_MARGIN,
                             yPos, 0.8f, 0.8f, 0.0f, settings.text_color, 0);
            yPos += scaledFontHeight;
        }
    }

    // Draw scroll indicator if scrolled
    if (consoleState.historyScroll > 0)
    {
        const float scrollIndColor[4] = {1.0f, 1.0f, 0.0f, 0.8f};
        char scrollText[64];
        _snprintf(scrollText, sizeof(scrollText), "[Scrolled up %d lines]", consoleState.historyScroll);
        R_AddCmdDrawText(scrollText, 64, iw4::mp::sharedUiInfo->assets.consoleFont, CONSOLE_X + CONSOLE_WIDTH - 200.0f,
                         CONSOLE_Y + 20.0f, 0.7f, 0.7f, 0.0f, scrollIndColor, 0);
    }

    // Draw input line separator
    yPos = CONSOLE_Y + CONSOLE_HEIGHT - 35.0f;
    const float separatorColor[4] = {0.3f, 0.3f, 0.3f, 1.0f};
    R_AddCmdDrawStretchPic(CONSOLE_X + TEXT_MARGIN, yPos, CONSOLE_WIDTH - (TEXT_MARGIN * 2), 1.0f, 0.0f, 0.0f, 1.0f,
                           1.0f, separatorColor, iw4::mp::sharedUiInfo->assets.whiteMaterial);

    // Draw prompt and input
    yPos += 10.0f;
    R_AddCmdDrawText("> ", 2, iw4::mp::sharedUiInfo->assets.consoleFont, CONSOLE_X + TEXT_MARGIN, yPos, 0.8f, 0.8f,
                     0.0f, settings.prompt_color, 0);

    // Draw input buffer
    R_AddCmdDrawText(consoleState.inputBuffer, MAX_INPUT_LENGTH, iw4::mp::sharedUiInfo->assets.consoleFont,
                     CONSOLE_X + TEXT_MARGIN + 20.0f, yPos, 0.8f, 0.8f, 0.0f, settings.input_text_color, 0);

    // Draw cursor (blinking)
    unsigned int currentTime = Sys_Milliseconds(); // You'll need to get actual game time here
    if (currentTime - consoleState.lastCursorBlink > CURSOR_BLINK_TIME)
    {
        consoleState.cursorVisible = !consoleState.cursorVisible;
        consoleState.lastCursorBlink = currentTime;
    }

    if (consoleState.cursorVisible)
    {
        // Calculate cursor position based on text width
        char tempBuffer[MAX_INPUT_LENGTH];
        strncpy(tempBuffer, consoleState.inputBuffer, consoleState.inputCursor);
        tempBuffer[consoleState.inputCursor] = '\0';

        float cursorX =
            CONSOLE_X + TEXT_MARGIN + 20.0f + (consoleState.inputCursor * 8.0f); // Approximate character width

        R_AddCmdDrawText("_", 1, iw4::mp::sharedUiInfo->assets.consoleFont, cursorX, yPos, 0.8f, 0.8f, 0.0f,
                         settings.input_text_color, 0);
    }
}

void Console::HandleInput()
{
    XINPUT_KEYSTROKE keystroke = {};
    uint32_t result = XInputGetKeystroke(0, XINPUT_FLAG_KEYBOARD, &keystroke);

    if (result == ERROR_EMPTY || result == ERROR_DEVICE_NOT_CONNECTED)
        return;

    if (result != ERROR_SUCCESS)
    {
        DbgPrint("Unexpected error while getting keystroke: %X.", result);
        return;
    }

    // Flags set to 0 means no keys were pressed at the current frame, we also ignore keyup events
    if (keystroke.Flags == 0 || keystroke.Flags & XINPUT_KEYSTROKE_KEYUP)
        return;

    DbgPrint("Key pressed: %c (VK: %d)", keystroke.Unicode, keystroke.VirtualKey);

    // Toggle console with tilde key
    if (keystroke.VirtualKey == VK_OEM_3)
    {
        consoleState.isOpen = !consoleState.isOpen;
        if (consoleState.isOpen)
        {
            // Clear input when opening
            memset(consoleState.inputBuffer, 0, sizeof(consoleState.inputBuffer));
            consoleState.inputCursor = 0;
        }
        return;
    }

    if (!consoleState.isOpen)
    {
        return;
    }

    // Handle Page Up - scroll history up
    if (keystroke.VirtualKey == VK_PRIOR) // VK_PRIOR is Page Up
    {
        int fontHeight = R_TextHeight(iw4::mp::sharedUiInfo->assets.consoleFont);
        float scaledFontHeight = fontHeight * 0.8f;
        int maxVisibleLines = (int)((CONSOLE_HEIGHT - 50.0f) / scaledFontHeight);
        int maxScroll = consoleState.lineCount - maxVisibleLines;

        if (maxScroll > 0)
        {
            // Scroll up by half a page
            consoleState.historyScroll += maxVisibleLines / 2;

            // Clamp to maximum scroll
            if (consoleState.historyScroll > maxScroll)
                consoleState.historyScroll = maxScroll;
        }
        return;
    }

    // Handle Page Down - scroll history down
    if (keystroke.VirtualKey == VK_NEXT) // VK_NEXT is Page Down
    {
        int fontHeight = R_TextHeight(iw4::mp::sharedUiInfo->assets.consoleFont);
        float scaledFontHeight = fontHeight * 0.8f;
        int maxVisibleLines = (int)((CONSOLE_HEIGHT - 50.0f) / scaledFontHeight);

        // Scroll down by half a page
        consoleState.historyScroll -= maxVisibleLines / 2;

        // Clamp to minimum (0 = bottom of history)
        if (consoleState.historyScroll < 0)
            consoleState.historyScroll = 0;

        return;
    }

    // Handle Home key - jump to top of history
    if (keystroke.VirtualKey == VK_HOME)
    {
        int fontHeight = R_TextHeight(iw4::mp::sharedUiInfo->assets.consoleFont);
        float scaledFontHeight = fontHeight * 0.8f;
        int maxVisibleLines = (int)((CONSOLE_HEIGHT - 50.0f) / scaledFontHeight);
        int maxScroll = consoleState.lineCount - maxVisibleLines;

        if (maxScroll > 0)
            consoleState.historyScroll = maxScroll;

        return;
    }

    // Handle End key - jump to bottom of history
    if (keystroke.VirtualKey == VK_END)
    {
        consoleState.historyScroll = 0;
        return;
    }

    if (keystroke.VirtualKey == VK_BACK)
    {
        if (consoleState.inputCursor > 0)
        {
            consoleState.inputCursor--;
            consoleState.inputBuffer[consoleState.inputCursor] = '\0';
        }
    }

    if (keystroke.VirtualKey == VK_RETURN)
    {
        if (strlen(consoleState.inputBuffer) > 0)
        {
            ProcessCommand(consoleState.inputBuffer);
            memset(consoleState.inputBuffer, 0, sizeof(consoleState.inputBuffer));
            consoleState.inputCursor = 0;
            consoleState.historyScroll = 0; // Reset scroll to bottom after command
        }
    }

    //
    // 0x40 : unassigned
    // VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
    //

    if (keystroke.Unicode && keystroke.VirtualKey >= 0x41 && keystroke.VirtualKey <= 0x5A) // Printable characters
    {
        if (consoleState.inputCursor < MAX_INPUT_LENGTH - 1)
        {
            consoleState.inputBuffer[consoleState.inputCursor++] = static_cast<char>(keystroke.Unicode);
            consoleState.inputBuffer[consoleState.inputCursor] = '\0';
        }
    }
}

// Public API functions
void Console::Open()
{
    consoleState.isOpen = true;
    memset(consoleState.inputBuffer, 0, sizeof(consoleState.inputBuffer));
    consoleState.inputCursor = 0;
}

void Console::Close()
{
    consoleState.isOpen = false;
}

void Console::Toggle()
{
    if (consoleState.isOpen)
    {
        Close();
    }
    else
    {
        Open();
    }
}

bool Console::IsOpen()
{
    return consoleState.isOpen;
}

void Console::ExecuteCommand(const char *command)
{
    ProcessCommand(command);
}

Detour Console::SCR_DrawScreenField_Detour;

void Console::SCR_DrawScreenField_Hook(int localClientNum, int refreshedUI)
{
    // Call the original function
    Console::SCR_DrawScreenField_Detour.GetOriginal<decltype(iw4::mp::SCR_DrawScreenField)>()(localClientNum,
                                                                                              refreshedUI);

    // TODO: Remove this hack once we figure out why the game crashes when rendering the console too early (text
    // rendering)
    // Track the start time (initialized only once)
    static DWORD startTime = GetTickCount();
    DWORD currentTime = GetTickCount();
    DWORD elapsedMs = currentTime - startTime;

    // Only render the console after x seconds have passed
    if (elapsedMs >= 2000)
    {
        HandleInput();
        RenderConsole();
    }
}

Detour Console::CL_ConsolePrint_Detour;
void Console::CL_ConsolePrint_Hook(int localClientNum, int channel, const char *txt, unsigned int duration,
                                   unsigned int pixelWidth, int flags)
{
    CL_ConsolePrint_Detour.GetOriginal<decltype(CL_ConsolePrint)>()(localClientNum, channel, txt, duration, pixelWidth,
                                                                    flags);
    // Add the message to the console history (circular buffer handles overflow)
    consoleState.AddToHistory(txt);
}

Console::Console()
{
    SCR_DrawScreenField_Detour = Detour(iw4::mp::SCR_DrawScreenField, SCR_DrawScreenField_Hook);
    SCR_DrawScreenField_Detour.Install();

    // Redirect the games internal console print to ours
    CL_ConsolePrint_Detour = Detour(CL_ConsolePrint, CL_ConsolePrint_Hook);
    CL_ConsolePrint_Detour.Install();
}

Console::~Console()
{
    SCR_DrawScreenField_Detour.Remove();

    CL_ConsolePrint_Detour.Remove();
}
