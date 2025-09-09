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

// Screen dimensions are static on the Xbox 360
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

const Console::Settings settings = {
    {1.0f, 1.0f, 1.0f, 1.0f}, // text_color
    {0.0f, 0.0f, 0.0f, 0.75f} // background_color
};

void Console::RenderConsole()
{
    R_AddCmdDrawStretchPic(5.0f, 5.0f, 1270.0f, 710.0f, 0.0f, 0.0f, 1.0f, 1.0f, settings.background_color,
                           iw4::mp::sharedUiInfo->assets.whiteMaterial);

    const char *text = "Rendering API Text";
    R_AddCmdDrawText(text, 64, iw4::mp::sharedUiInfo->assets.consoleFont, 90.0f, 375.0f, 1.0f, 1.0f, 0.0f,
                     settings.text_color, 0);
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
