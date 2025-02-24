#include <cstddef>

namespace game
{
    namespace mp
    {

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

        struct field_t
        {
            int cursor;
            int scroll;
            int drawWidth;
            int widthInPixels;
            float charHeight;
            int fixedSize;
            char buffer[256];
        };

        struct KeyState
        {
            int down;
            int repeats;
            const char *binding;
        };

        enum LocSelInputState
        {
            LOC_SEL_INPUT_NONE = 0x0,
            LOC_SEL_INPUT_CONFIRM = 0x1,
            LOC_SEL_INPUT_CANCEL = 0x2,
        };

        struct PlayerKeyState
        {
            field_t chatField;
            int chat_team;
            int overstrikeMode;
            int anyKeyDown;
            KeyState keys[256];
            LocSelInputState locSelInputState;
        };

        struct keyname_t
        {
            const char *name;
            int keynum;
        };

        struct ConDrawInputGlob
        {
            char autoCompleteChoice[64];
            int matchIndex;
            int matchCount;
            const char *inputText;
            int inputTextLen;
            bool hasExactMatch;
            bool mayAutoComplete;
            float x;
            float y;
            float leftX;
            float fontHeight;
        };

        enum DvarSetSource
        {
            DVAR_SOURCE_INTERNAL = 0x0,
            DVAR_SOURCE_EXTERNAL = 0x1,
            DVAR_SOURCE_SCRIPT = 0x2,
        };

        union DvarValue
        {
            bool enabled;
            int integer;
            unsigned int unsignedInt;
            float value;
            float vector[4];
            const char *string;
            unsigned __int8 color[4];
        };

        union DvarLimits
        {
            struct
            {
                int stringCount;
                const char **strings;
            } enumeration;

            struct
            {
                int min;
                int max;
            } integer;

            struct
            {
                float min;
                float max;
            } value;

            struct
            {
                float min;
                float max;
            } vector;
        };

        struct dvar_s
        {
            const char *name;
            const char *description;
            unsigned __int16 flags;
            unsigned __int8 type;
            bool modified;
            DvarValue current;
            DvarValue latched;
            DvarValue reset;
            DvarLimits domain;
            dvar_s *hashNext;
        };

        struct clientStatic_t
        {
            int quit;
            int hunkUsersStarted;
            char servername[256];
            _XSESSION_INFO hostInfo;
            int rendererStarted;
            int soundStarted;
            int uiStarted;
            int frametime;
            int realtime;
            int realFrametime;
            // clientLogo_t logo;
            // float mapCenter[3];
            // int numlocalservers;
            // serverInfo_t localServers[16];
            // int pingUpdateSource;
            // Material *whiteMaterial;
            // Material *consoleMaterial;
            // Font_s *consoleFont;
            // vidConfig_t vidConfig;
            // clientDebug_t debug;
            // XNADDR xnaddrs[24];
            // volatile int scriptError;
            // float debugRenderPos[3];
        };

        struct Material;

        struct rectDef_s
        {
            float x;
            float y;
            float w;
            float h;
            int horzAlign;
            int vertAlign;
        };

        struct windowDef_t
        {
            const char *name;
            rectDef_s rect;
            rectDef_s rectClient;
            const char *group;
            int style;
            int border;
            int ownerDraw;
            int ownerDrawFlags;
            float borderSize;
            int staticFlags;
            int dynamicFlags[4];
            int nextTime;
            float foreColor[4];
            float backColor[4];
            float borderColor[4];
            float outlineColor[4];
            Material *background;
        };

        struct expressionEntry;

        struct statement_s
        {
            int numEntries;
            expressionEntry **entries;
        };

        struct snd_alias_list_t;

        struct columnInfo_s
        {
            int pos;
            int width;
            int maxChars;
            int alignment;
        };

        struct listBoxDef_s
        {
            int startPos[4];
            int endPos[4];
            int drawPadding;
            float elementWidth;
            float elementHeight;
            int elementStyle;
            int numColumns;
            columnInfo_s columnInfo[16];
            const char *doubleClick;
            int notselectable;
            int noScrollBars;
            int usePaging;
            float selectBorder[4];
            float disableColor[4];
            Material *selectIcon;
        };

        struct editFieldDef_s
        {
            float minVal;
            float maxVal;
            float defVal;
            float range;
            int maxChars;
            int maxCharsGotoNext;
            int maxPaintChars;
            int paintOffset;
        };

        struct multiDef_s
        {
            const char *dvarList[32];
            const char *dvarStr[32];
            float dvarValue[32];
            int count;
            int strDef;
        };

        union itemDefData_t
        {
            listBoxDef_s *listBox;
            editFieldDef_s *editField;
            multiDef_s *multi;
            const char *enumDvarName;
            void *data;
        };

        struct menuDef_t; // Forward declaration
        struct ItemKeyHandler
        {
            int key;
            const char *action;
            ItemKeyHandler *next;
        };

        struct itemDef_s
        {
            windowDef_t window;
            rectDef_s textRect[4];
            int type;
            int dataType;
            int alignment;
            int fontEnum;
            int textAlignMode;
            float textalignx;
            float textaligny;
            float textscale;
            int textStyle;
            int gameMsgWindowIndex;
            int gameMsgWindowMode;
            const char *text;
            int itemFlags;
            menuDef_t *parent;
            const char *mouseEnterText;
            const char *mouseExitText;
            const char *mouseEnter;
            const char *mouseExit;
            const char *action;
            const char *onAccept;
            const char *onFocus;
            const char *leaveFocus;
            const char *dvar;
            const char *dvarTest;
            ItemKeyHandler *onKey;
            const char *enableDvar;
            int dvarFlags;
            snd_alias_list_t *focusSound;
            float special;
            int cursorPos[4];
            itemDefData_t typeData;
            int imageTrack;
            statement_s visibleExp;
            statement_s textExp;
            statement_s materialExp;
            statement_s rectXExp;
            statement_s rectYExp;
            statement_s rectWExp;
            statement_s rectHExp;
            statement_s forecolorAExp;
        };

        struct menuDef_t
        {
            windowDef_t window;
            const char *font;
            int fullScreen;
            int itemCount;
            int fontIndex;
            int cursorItem[4];
            int fadeCycle;
            float fadeClamp;
            float fadeAmount;
            float fadeInAmount;
            float blurRadius;
            const char *onOpen;
            const char *onClose;
            const char *onESC;
            ItemKeyHandler *onKey;
            statement_s visibleExp;
            const char *allowedBinding;
            const char *soundName;
            int imageTrack;
            float focusColor[4];
            float disableColor[4];
            statement_s rectXExp;
            statement_s rectYExp;
            itemDef_s **items;
        };

        enum uiMenuCommand_t
        {
            UIMENU_NONE = 0x0,
            UIMENU_MAIN = 0x1,
            UIMENU_INGAME = 0x2,
            UIMENU_PREGAME = 0x3,
            UIMENU_POSTGAME = 0x4,
            UIMENU_WM_QUICKMESSAGE = 0x5,
            UIMENU_SCRIPT_POPUP = 0x6,
            UIMENU_SCOREBOARD = 0x7,
            UIMENU_SPLITSCREENGAMESETUP = 0x8,
            UIMENU_SYSTEMLINKJOINGAME = 0x9,
            UIMENU_PARTY = 0xA,
            UIMENU_GAMELOBBY = 0xB,
            UIMENU_PRIVATELOBBY = 0xC,
        };

        enum UILocalVarType
        {
            UILOCALVAR_INT = 0x0,
            UILOCALVAR_FLOAT = 0x1,
            UILOCALVAR_STRING = 0x2,
        };

        struct UILocalVar
        {
            UILocalVarType type;
            const char *name;
            union
            {

                int integer;
                float value;
                const char *string;

            } u;
        };

        struct UILocalVarContext
        {
            UILocalVar table[256];
        };

        struct UiContext
        {
            int localClientNum;
            float bias;
            int realTime;
            int frameTime;
            struct
            {
                float x;
                float y;
            } cursor;
            int isCursorVisible;
            int screenWidth;
            int screenHeight;
            float screenAspect;
            float FPS;
            float blurRadiusOut;
            menuDef_t *Menus[512];
            int menuCount;
            menuDef_t *menuStack[16];
            int openMenuCount;
            UILocalVarContext localVars;
        };

        struct __declspec(align(4)) uiInfo_s
        {
            UiContext uiDC;
            int myTeamCount;
            int playerRefresh;
            int playerIndex;
            int timeIndex;
            int previousTimes[4];
            uiMenuCommand_t currentMenuType;
            bool allowScriptMenuResponse;
        };

        struct ScreenPlacement
        {
            float scaleVirtualToReal[2];
            float scaleVirtualToFull[2];
            float scaleRealToVirtual[2];
            float virtualViewableMin[2];
            float virtualViewableMax[2];
            float realViewportSize[2];
            float realViewableMin[2];
            float realViewableMax[2];
            float subScreenLeft;
        };

        enum sysEventType_t
        {
            SE_NONE = 0x0,
            SE_CHAR = 0x1,
            SE_CONSOLE = 0x2,
        };

        struct sysEvent_t
        {
            int evPortIndex;
            int evTime;
            sysEventType_t evType;
            int evValue;
            int evValue2;
            int evPtrLength;
            void *evPtr;
        };

        struct cmd_function_s
        {
            cmd_function_s *next;
            const char *name;
            const char *autoCompleteDir;
            const char *autoCompleteExt;
            void (*function)();
        };

        typedef void (*Cbuf_AddText_t)(int localClientNum, const char *text);
        typedef void (*CL_CharEvent_t)(int localClientNum, int key);
        typedef void (*CL_ConsoleCharEvent_t)(int localClientNum, int key);
        typedef void (*CL_GamepadButtonEvent_t)(int localClientNum, int controllerIndex, int key, int down, unsigned int time);
        typedef void (*CL_GamepadButtonEventForPort_t)(int portIndex, int key, int down, unsigned int time);
        typedef int (*Com_GetLocalClientNumForEventOnPort_t)(int portIndex);
        typedef void (*IN_GamepadsMove_t)();
        typedef int (*Key_SetCatcher_t)(int localClientNum, int catcher);
        typedef unsigned int (*Sys_Milliseconds_t)();
        typedef int (*Con_CancelAutoComplete_t)();
        typedef void (*Con_Close_t)(int localClientNum);
        typedef void (*Com_Printf_t)(int channel, const char *fmt, ...);
        typedef int (*Com_sprintf_t)(char *dest, unsigned int size, const char *fmt, ...);
        typedef void (*Dvar_SetStringFromSource_t)(dvar_s *dvar, const char *string, DvarSetSource source);
        typedef int (*I_strnicmp_t)(const char *s0, const char *s1, int n);
        typedef int (*UI_KeyEvent_t)(int localClientNum, int key, int down);
        typedef int (*UI_SetActiveMenu_t)(int localClientNum, int menu);
        typedef void (*CL_StopLogoOrCinematic_t)(int localClientNum);
        typedef void (*CL_DisconnectLocalClient_t)(int localClientNum);
        typedef int (*CL_AllLocalClientsDisconnected_t)();
        typedef void (*SCR_UpdateScreen_t)();
        typedef void (*Field_AdjustScroll_t)(const ScreenPlacement *scrPlace, field_t *edit);
        typedef int (*Con_AnySpaceAfterCommand_t)();
        typedef int (*Scoreboard_HandleInputXboxLive_t)(unsigned int localClientNum, int key);
        typedef int (*Scoreboard_HandleInputNonXboxLive_t)(int localClientNum, int key);
        typedef void (*Menu_HandleKey_t)(UiContext *dc, menuDef_t *menu, int key, int down);
        typedef menuDef_t *(*Menu_GetFocused_t)(UiContext *dc);
        typedef sysEvent_t *(*Sys_GetEvent_t)(sysEvent_t *result);
        typedef void (*CL_Input_t)(int localClientNum);
    }
}
