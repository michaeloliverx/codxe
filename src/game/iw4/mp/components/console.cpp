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
#define MAX_HISTORY_LINES 20
#define CURSOR_BLINK_TIME 500 // milliseconds

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
    char history[MAX_HISTORY_LINES][MAX_INPUT_LENGTH];
    int historyCount;
    int historyScroll;
    unsigned int lastCursorBlink;
    bool cursorVisible;

    ConsoleState()
        : isOpen(false), inputCursor(0), historyCount(0), historyScroll(0), lastCursorBlink(0), cursorVisible(true)
    {
        memset(inputBuffer, 0, sizeof(inputBuffer));
        memset(history, 0, sizeof(history));

        // Add welcome message
        AddToHistory("Console initialized. Type 'help' for commands.");
    }

    void AddToHistory(const char *text)
    {
        if (historyCount < MAX_HISTORY_LINES)
        {
            strncpy(history[historyCount], text, MAX_INPUT_LENGTH - 1);
            historyCount++;
        }
        else
        {
            // Shift history up
            for (int i = 0; i < MAX_HISTORY_LINES - 1; i++)
            {
                strncpy(history[i], history[i + 1], MAX_INPUT_LENGTH - 1);
            }
            strncpy(history[MAX_HISTORY_LINES - 1], text, MAX_INPUT_LENGTH - 1);
        }
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
        consoleState.historyCount = 0;
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

    float yPos = CONSOLE_Y + TEXT_MARGIN;

    // Draw history (from bottom up, leaving space for input)
    int startLine = 0;
    int maxVisibleLines = (int)((CONSOLE_HEIGHT - 50.0f) / LINE_HEIGHT);

    if (consoleState.historyCount > maxVisibleLines)
    {
        startLine = consoleState.historyCount - maxVisibleLines;
    }

    for (int i = startLine; i < consoleState.historyCount; i++)
    {
        R_AddCmdDrawText(consoleState.history[i], MAX_INPUT_LENGTH, iw4::mp::sharedUiInfo->assets.consoleFont,
                         CONSOLE_X + TEXT_MARGIN, yPos, 0.8f, 0.8f, 0.0f, settings.text_color, 0);
        yPos += LINE_HEIGHT;
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

    // Render the console on top of everything else
    // RenderConsole();

    // TODO: Remove this hack once we figure out why the game crashes when rendering the console too early (text
    // rendering)

    // Track the start time (initialized only once)
    static DWORD startTime = GetTickCount();

    // Check if at least 5 seconds (5000 milliseconds) have passed
    DWORD currentTime = GetTickCount();
    DWORD elapsedMs = currentTime - startTime;

    // Only render the console after 5 seconds
    if (elapsedMs >= 5000)
    {
        HandleInput();
        RenderConsole();
    }
}

Console::Console()
{
    SCR_DrawScreenField_Detour = Detour(iw4::mp::SCR_DrawScreenField, SCR_DrawScreenField_Hook);
    SCR_DrawScreenField_Detour.Install();
}

Console::~Console()
{
    SCR_DrawScreenField_Detour.Remove();
}
