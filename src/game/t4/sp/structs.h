#pragma once

namespace t4
{
    namespace sp
    {
        // usercmd_t->button bits
        enum button_mask
        {
            KEY_FIRE = 0x1,
            KEY_SPRINT = 0x2,
            KEY_MELEE = 0x4,
            KEY_USE = 0x8,
            KEY_RELOAD = 0x10,
            KEY_USERELOAD = 0x20,
            KEY_LEANLEFT = 0x40,
            KEY_LEANRIGHT = 0x80,
            KEY_PRONE = 0x100,
            KEY_CROUCH = 0x200,
            KEY_GOSTAND = 0x400,
            KEY_ADSMODE = 0x800,
            KEY_TEMP = 0x1000,
            KEY_HOLDBREATH = 0x2000,
            KEY_FRAG = 0x4000,
            KEY_SMOKE = 0x8000,
            KEY_SELECTING_LOCATION = 0x10000,
            KEY_CANCEL_LOCATION = 0x20000,
            KEY_NIGHTVISION = 0x40000,
            KEY_ADS = 0x80000,
            KEY_REVERSE = 0x100000,
            KEY_HANDBRAKE = 0x200000,
            KEY_THROW = 0x400000,
            KEY_INMENU = 0x800000,
        };

        struct gclient_s
        {
            char pad_0[8780];
            int buttons; // Offset 8780
            int oldbuttons;
            int latched_buttons;       // Offset 8784
            int buttonsSinceLastFrame; // Offset 8792
            char pad_1[240];
        };
        static_assert(sizeof(gclient_s) == 9036, "");
        static_assert(offsetof(gclient_s, buttons) == 8780, "");
        static_assert(offsetof(gclient_s, buttonsSinceLastFrame) == 8792, "");

        struct gentity_s
        {
            char pad_0[384];   //
            gclient_s *client; // OFS: 384 SIZE: 0x4
            char pad_1[48];    //
            int flags;         // OFS: 436 SIZE: 0x4
            char pad_2[448];   //
        };
        static_assert(sizeof(gentity_s) == 888, "");
        static_assert(offsetof(gentity_s, client) == 384, "");
        static_assert(offsetof(gentity_s, flags) == 436, "");

        struct usercmd_s
        {
            int serverTime;   // 0x00
            int buttons;      // 0x04
            char pad[14];     //
            char forwardmove; // OFS: 0x16 SIZE: 0x1
            char rightmove;   // OFS: 0x17 SIZE: 0x1
            char pad2[32];    // 0x08
        };
        static_assert(sizeof(usercmd_s) == 56, "");
        static_assert(offsetof(usercmd_s, serverTime) == 0x00, "");
        static_assert(offsetof(usercmd_s, buttons) == 4, "");
        static_assert(offsetof(usercmd_s, forwardmove) == 0x16, "");
        static_assert(offsetof(usercmd_s, rightmove) == 0x17, "");

        struct __declspec(align(4)) client_t
        {
            char pad[69880];
            usercmd_s lastUsercmd;
            char pad2[283340];
        };
        static_assert(sizeof(client_t) == 353276, "");
        static_assert(offsetof(client_t, lastUsercmd) == 69880, "");

        enum scriptInstance_t : __int32
        {
            SCRIPTINSTANCE_SERVER = 0x0,
            SCRIPTINSTANCE_CLIENT = 0x1,
            SCRIPT_INSTANCE_MAX = 0x2,
        };

#pragma warning(disable : 4324)
        struct __declspec(align(8)) scr_entref_t
        {
            uint16_t entnum;
            uint16_t classnum;
            uint16_t localclientnum;
        };
#pragma warning(default : 4324)
        static_assert(sizeof(scr_entref_t) == 8, "");
        static_assert(offsetof(scr_entref_t, entnum) == 0x0, "");
        static_assert(offsetof(scr_entref_t, classnum) == 0x2, "");
        static_assert(offsetof(scr_entref_t, localclientnum) == 0x4, "");

        typedef void (*BuiltinFunction)();

        struct BuiltinFunctionDef
        {
            const char *actionString;   // OFS: 0x0 SIZE: 0x4
            BuiltinFunction actionFunc; // OFS: 0x4 SIZE: 0x4
            int type;                   // OFS: 0x8 SIZE: 0x4
        };
        static_assert(sizeof(BuiltinFunctionDef) == 0xC, "");
        static_assert(offsetof(BuiltinFunctionDef, actionString) == 0x0, "");
        static_assert(offsetof(BuiltinFunctionDef, actionFunc) == 0x4, "");
        static_assert(offsetof(BuiltinFunctionDef, type) == 0x8, "");

        typedef void (*BuiltinMethod)(scr_entref_t entref);

    }
}
