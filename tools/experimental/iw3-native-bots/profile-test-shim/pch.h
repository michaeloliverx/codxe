#pragma once

// Minimal engine/SDK boundary for compiling the actual bot_native.cpp on the host.
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

struct LARGE_INTEGER
{
    long long QuadPart;
};

namespace profile_test
{
inline long long ticks = 0;
inline unsigned int numParams = 0;
inline int params[5] = {};
inline int profileRequested = 1;
inline float vectors[5][3] = {};
inline std::vector<double> returned;
inline std::vector<std::string> messages;
inline int lastFrameTime;
inline void (*duringFrame)() = nullptr;
inline void ServerFrame(int levelTime)
{
    lastFrameTime = levelTime;
    if (duringFrame)
        duringFrame();
    ticks += 40;
}
} // namespace profile_test

inline bool QueryPerformanceCounter(LARGE_INTEGER *value)
{
    value->QuadPart = profile_test::ticks;
    return true;
}
inline bool QueryPerformanceFrequency(LARGE_INTEGER *value)
{
    value->QuadPart = 1000000;
    return true;
}
inline void ZeroMemory(void *data, size_t size)
{
    memset(data, 0, size);
}

inline void DbgPrint(const char *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    profile_test::messages.push_back(buffer);
}

class Detour
{
  public:
    Detour()
    {
    }
    template <typename T> Detour(void *, T)
    {
    }
    void Install(size_t minimumLength = 0)
    {
        if (minimumLength != 20)
            throw std::runtime_error("G_RunFrame must copy through its stack allocation");
    }
    void Remove()
    {
    }
    template <typename T> T GetOriginal()
    {
        return reinterpret_cast<T>(&profile_test::ServerFrame);
    }
};

class Config
{
  public:
    static const char *GetActiveMod()
    {
        return "bot_warfare_native";
    }
};

namespace iw3
{
namespace mp
{
enum
{
    MAX_CLIENTS = 24,
    NA_BOT = 4,
    CS_ACTIVE = 4,
    SESS_STATE_PLAYING = 0,
    SESS_STATE_DEAD = 1,
    TEAM_SPECTATOR = 3,
    KEY_MASK_FIRE = 1, KEY_MASK_SPRINT = 2, KEY_MASK_RELOAD = 16,
    KEY_MASK_PRONE = 256, KEY_MASK_CROUCH = 512, KEY_MASK_JUMP = 1024,
    KEY_MASK_ADS_MODE = 2048, KEY_MASK_ADS = 524288
};
struct scr_entref_t
{
    unsigned short entnum, classnum;
};
typedef void (*BuiltinFunction)();
typedef void (*BuiltinMethod)(scr_entref_t);
struct playerState_s
{
    float velocity[3], viewangles[3], origin[3], viewHeightCurrent;
    int pm_type, pm_flags;
};
struct usercmd_s
{
    int angles[3], buttons;
    char forwardmove, rightmove;
};
struct gclient_s
{
    playerState_s ps;
    struct
    {
        int archiveTime, sessionState;
        struct { int team; } cs;
        usercmd_s cmd;
    } sess;
    int bFrozen;
};
struct gentity_s
{
    gclient_s *client;
    int health;
};
struct client_t
{
    gentity_s *gentity;
    struct
    {
        struct
        {
            struct
            {
                int type;
            } remoteAddress;
        } netchan;
        int state;
    } header;
};
struct serverStaticHeader_t
{
    client_t *clients;
    int time;
    int maxclients;
};
inline client_t testClients[MAX_CLIENTS] = {};
inline serverStaticHeader_t testServer = {testClients, 0, MAX_CLIENTS};
inline serverStaticHeader_t *svsHeader = &testServer;
inline gentity_s g_entities[MAX_CLIENTS] = {};

inline void GetPlayerEntity(scr_entref_t)
{
}
inline void Scr_Error(const char *message)
{
    throw std::runtime_error(message);
}
inline void Scr_ObjectError(const char *message)
{
    throw std::runtime_error(message);
}
inline void Scr_ParamError(unsigned int, const char *message)
{
    throw std::runtime_error(message);
}
inline unsigned int Scr_GetNumParam()
{
    return profile_test::numParams;
}
inline int Scr_GetInt(unsigned int param)
{
    return profile_test::params[param];
}
inline float Scr_GetFloat(unsigned int param)
{
    return static_cast<float>(profile_test::params[param]);
}
inline void Scr_GetVector(unsigned int param, float *value)
{
    memcpy(value, profile_test::vectors[param], sizeof(float) * 3);
}
inline void Scr_AddInt(int value)
{
    profile_test::returned.push_back(value);
}
inline void Scr_AddFloat(float value)
{
    profile_test::returned.push_back(value);
}
inline void Scr_AddUndefined()
{
}
inline void Scr_MakeArray()
{
    profile_test::returned.clear();
}
inline void Scr_AddArray()
{
}
inline void SetClientViewAngle(gentity_s *, float *)
{
}
inline const char *Dvar_GetString(const char *)
{
    return "mp_crash";
}
inline int Dvar_GetInt(const char *name)
{
    if (!strcmp(name, "bots_native_profile"))
        return profile_test::profileRequested;
    if (!strcmp(name, "bots_main_awareness_quality"))
        return 2;
    if (!strcmp(name, "sv_fps"))
        return 20;
    if (!strcmp(name, "bots_native_sight_budget")) return 6;
    if (!strcmp(name, "bots_native_path_budget")) return 1;
    if (!strcmp(name, "bots_native_reaction_ms")) return 250;
    if (!strcmp(name, "bots_native_range")) return 4000;
    if (!strcmp(name, "bots_native_fov")) return 180;
    if (!strcmp(name, "bots_native_turn_rate")) return 300;
    return 1;
}
} // namespace mp
} // namespace iw3
