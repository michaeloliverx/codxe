#pragma warning(disable : 4480)

namespace mp
{

    struct cmd_function_s
    {
        cmd_function_s *next;
        const char *name;
        const char *autoCompleteDir;
        const char *autoCompleteExt;
        void(__fastcall *function)();
    };

    enum keyNum_t : __int32
    {
        K_NONE = 0x0,
        K_FIRSTGAMEPADBUTTON_RANGE_1 = 0x1,
        K_BUTTON_A = 0x1,
        K_BUTTON_B = 0x2,
        K_BUTTON_X = 0x3,
        K_BUTTON_Y = 0x4,
        K_BUTTON_LSHLDR = 0x5,
        K_BUTTON_RSHLDR = 0x6,
        K_LASTGAMEPADBUTTON_RANGE_1 = 0x6,
        K_TAB = 0x9,
        K_ENTER = 0xD,
        K_FIRSTGAMEPADBUTTON_RANGE_2 = 0xE,
        K_BUTTON_START = 0xE,
        K_BUTTON_BACK = 0xF,
        K_BUTTON_LSTICK = 0x10,
        K_BUTTON_RSTICK = 0x11,
        K_BUTTON_LTRIG = 0x12,
        K_BUTTON_RTRIG = 0x13,
        K_DPAD_UP = 0x14,
        K_FIRSTDPAD = 0x14,
        K_DPAD_DOWN = 0x15,
        K_DPAD_LEFT = 0x16,
        K_DPAD_RIGHT = 0x17,
        K_BUTTON_LSTICK_ALTIMAGE = 0xBC,
        K_BUTTON_RSTICK_ALTIMAGE = 0xBD,
        K_LASTDPAD = 0x17,
        K_LASTGAMEPADBUTTON_RANGE_2 = 0x17,
        K_ESCAPE = 0x1B,
        K_FIRSTGAMEPADBUTTON_RANGE_3 = 0x1C,
        K_APAD_UP = 0x1C,
        K_FIRSTAPAD = 0x1C,
        K_APAD_DOWN = 0x1D,
        K_APAD_LEFT = 0x1E,
        K_APAD_RIGHT = 0x1F,
        K_LASTAPAD = 0x1F,
        K_LASTGAMEPADBUTTON_RANGE_3 = 0x1F,
        K_SPACE = 0x20,
        K_BACKSPACE = 0x7F,
        K_ASCII_FIRST = 0x80,
        K_ASCII_181 = 0x80,
        K_ASCII_191 = 0x81,
        K_ASCII_223 = 0x82,
        K_ASCII_224 = 0x83,
        K_ASCII_225 = 0x84,
        K_ASCII_228 = 0x85,
        K_ASCII_229 = 0x86,
        K_ASCII_230 = 0x87,
        K_ASCII_231 = 0x88,
        K_ASCII_232 = 0x89,
        K_ASCII_233 = 0x8A,
        K_ASCII_236 = 0x8B,
        K_ASCII_241 = 0x8C,
        K_ASCII_242 = 0x8D,
        K_ASCII_243 = 0x8E,
        K_ASCII_246 = 0x8F,
        K_ASCII_248 = 0x90,
        K_ASCII_249 = 0x91,
        K_ASCII_250 = 0x92,
        K_ASCII_252 = 0x93,
        K_END_ASCII_CHARS = 0x94,
        K_COMMAND = 0x96,
        K_CAPSLOCK = 0x97,
        K_POWER = 0x98,
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
        K_F13 = 0xB3,
        K_F14 = 0xB4,
        K_F15 = 0xB5,
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
        K_MOUSE1 = 0xC8,
        K_MOUSE2 = 0xC9,
        K_MOUSE3 = 0xCA,
        K_MOUSE4 = 0xCB,
        K_MOUSE5 = 0xCC,
        K_MWHEELDOWN = 0xCD,
        K_MWHEELUP = 0xCE,
        K_AUX1 = 0xCF,
        K_AUX2 = 0xD0,
        K_AUX3 = 0xD1,
        K_AUX4 = 0xD2,
        K_AUX5 = 0xD3,
        K_AUX6 = 0xD4,
        K_AUX7 = 0xD5,
        K_AUX8 = 0xD6,
        K_AUX9 = 0xD7,
        K_AUX10 = 0xD8,
        K_AUX11 = 0xD9,
        K_AUX12 = 0xDA,
        K_AUX13 = 0xDB,
        K_AUX14 = 0xDC,
        K_AUX15 = 0xDD,
        K_AUX16 = 0xDE,
        K_LAST_KEY = 0xDF,
    };

    struct MapEnts
    {
        const char *name;
        char *entityString;
        int numEntityChars;
    };

    typedef void (*Cbuf_AddText_t)(int localClientNum, const char *text);

    typedef void (*CL_ConsolePrint_t)(int localClientNum, int channel, const char *txt, int duration, int pixelWidth, int flags);
    typedef void (*CL_GamepadButtonEvent_t)(int localClientNum, int controllerIndex, int key, int down, unsigned int time);

    typedef void (*Cmd_AddCommandInternal_t)(const char *cmdName, void (*function)(), cmd_function_s *allocedCmd);

    typedef void (*Com_Printf_t)(int channel, const char *fmt, ...);

    typedef void (*Load_MapEntsPtr_t)();

    typedef char *(*Scr_ReadFile_FastFile_t)(const char *filename, const char *extFilename, const char *codePos, bool archive);
}

#pragma warning(default : 4480)
