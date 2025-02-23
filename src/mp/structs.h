#include <cstddef>

namespace game
{
    namespace mp
    {
        // struct cmd_function_s
        // {
        //     cmd_function_s *next;
        //     const char *name;
        //     const char *autoCompleteDir;
        //     const char *autoCompleteExt;
        //     void (*function)();
        // };

        // typedef void (*Cbuf_AddText_t)(int localClientNum, const char *text);
        // typedef void (*CG_GameMessage_t)(int localClientNum, const char *msg);
        // typedef void (*Dvar_SetIntByName_t)(const char *dvarName, int value);
        // typedef int (*Key_IsDown_t)(int localClientNum, int keynum);
        // typedef int (*Key_SetCatcher_t)(int localClientNum, int catcher);
        // typedef void (*RB_EndSceneRendering_t)(int *context, const int *input, const int *viewInfo);
        // typedef void (*SCR_DrawScreenField_t)(int localClientNum, int refreshedUI);
        // typedef char *(*Scr_ReadFile_FastFile_t)(const char *filename, const char *extFilename, const char *codePos, bool archive);
        // typedef void (*UI_RunMenuScript_t)(int localClientNum, const char **args, const char *actualScript);

        /* 9894 */
        struct GamePad
        {
            bool enabled;
            bool keyboardEnabled;
            __int16 digitals;
            __int16 lastDigitals;
            float analogs[8];
            float lastAnalogs[8];
            float sticks[4];
            float lastSticks[4];
            bool stickDown[4][2];
            bool stickDownLast[4][2];
            float lowRumble;
            float highRumble;
            struct
            {
                _XINPUT_VIBRATION Rumble;
            } feedback;
            _XINPUT_CAPABILITIES caps;
            _XINPUT_CAPABILITIES keyboardCaps;
        };
        static_assert(sizeof(GamePad) == 0xAC, "");

        // static_assert(offsetof(gclient_s, name) == 8688, "");
        // static_assert(offsetof(gclient_s, buttons) == 8804, "");
        // static_assert(offsetof(gclient_s, buttonsSinceLastFrame) == 8808, "");

        struct MessageLine
        {
            int messageIndex;
            int textBufPos;
            int textBufSize;
            int typingStartTime;
            int lastTypingSoundTime;
            int flags;
        };

        struct Message
        {
            int startTime;
            int endTime;
        };

        struct MessageWindow
        {
            MessageLine *lines;
            Message *messages;
            char *circularTextBuffer;
            int textBufSize;
            int lineCount;
            int padding;
            int scrollTime;
            int fadeIn;
            int fadeOut;
            int textBufPos;
            int firstLineIndex;
            int activeLineCount;
            int messageIndex;
        };

        struct MessageBuffer
        {
            char gamemsgText[4][2048];
            MessageWindow gamemsgWindows[4];
            MessageLine gamemsgLines[4][12];
            Message gamemsgMessages[4][12];
            char miniconText[4096];
            MessageWindow miniconWindow;
            MessageLine miniconLines[100];
            Message miniconMessages[100];
            char errorText[1024];
            MessageWindow errorWindow;
            MessageLine errorLines[5];
            Message errorMessages[5];
        };

        struct Console
        {
            int initialized;
            MessageWindow consoleWindow;
            MessageLine consoleLines[1024];
            Message consoleMessages[1024];
            char consoleText[32768];
            char textTempLine[512];
            unsigned int lineOffset;
            int displayLineOffset;
            int prevChannel;
            bool outputVisible;
            int fontHeight;
            int visibleLineCount;
            int visiblePixelWidth;
            float screenMin[2];
            float screenMax[2];
            MessageBuffer messageBuffer[4];
            float color[4];
        };

        enum connstate_t
        {
            CA_DISCONNECTED = 0x0,
            CA_CINEMATIC = 0x1,
            CA_LOGO = 0x2,
            CA_CONNECTING = 0x3,
            CA_CHALLENGING = 0x4,
            CA_CONNECTED = 0x5,
            CA_SENDINGSTATS = 0x6,
            CA_LOADING = 0x7,
            CA_PRIMED = 0x8,
            CA_ACTIVE = 0x9,
        };

        struct clientUIActive_t
        {
            bool active;
            bool isRunning;
            bool cgameInitialized;
            bool cgameInitCalled;
            bool mapPreloaded;
            int keyCatchers;
            bool displayHUDWithKeycatchUI;
            connstate_t connectionState;
            int nextScrollTime;
            bool invited;
            int numVoicePacketsSent;
            int numVoicePacketsSentStart;
        };

        enum sysEventType_t
        {
            SE_NONE = 0x0,
            SE_CHAR = 0x1,
            SE_CONSOLE = 0x2,
        };

        typedef void (*CL_ConsoleCharEvent_t)(int localClientNum, int key);
        typedef void (*CL_GamepadButtonEvent_t)(int localClientNum, int controllerIndex, int key, int down, int time);
        typedef void (*CL_GamepadButtonEventForPort_t)(int portIndex, int key, int down, unsigned int time);
        typedef int (*Com_GetLocalClientNumForEventOnPort_t)(int portIndex);
        typedef void (*IN_GamepadsMove_t)();
        typedef int (*Key_SetCatcher_t)(int localClientNum, int catcher);
        typedef unsigned int (*Sys_Milliseconds_t)();

    }
}
