#include "pch.h"
#include "bot_native.h"
#include "bot_native_core.h"
#include "bot_native_controller.h"

namespace iw3
{
namespace mp
{
namespace bot_native
{
namespace
{
iw3_bot_native::Graph graph;
iw3_bot_native::Aim aims[MAX_CLIENTS];
gentity_s *aimOwners[MAX_CLIENTS];
std::vector<int> resultPath;
typedef iw3_bot_native::ControllerFor<MAX_CLIENTS> NativeController;
static_assert(static_cast<int>(NativeController::Clients) == MAX_CLIENTS,
              "native bot state must cover every engine client slot");
NativeController controller;
bool controllerPaused = true;
bool controllerDrive = true;
int controllerFrame = -1;
int controllerConfigTime;
NativeController::Actor controllerActors[MAX_CLIENTS];

enum
{
    TargetTags = 7,
    TargetBulletTrace = 8,
    TargetSightTrace = 9,
    TargetBulletPassed = 10,
    TargetOther = 11,
    BotInput = 12,
    NativeControllerProfile = 13,
    NativeSight = 14,
    ProfileSlots = 15
};
bool profilingEnabled;
Detour serverFrameDetour;
struct Profile
{
    LARGE_INTEGER start;
    unsigned __int64 total;
    unsigned __int64 maximum;
    unsigned int calls;
    bool running;
};
Profile profiles[ProfileSlots];
unsigned __int64 profileWindowStart;
unsigned int profileGeneration;
unsigned int skippedBoundaryFrames;
unsigned __int64 targetPartsAtStart;
BuiltinMethod originalGetTagOrigin;
BuiltinFunction originalBulletTrace;
BuiltinFunction originalSightTracePassed;
BuiltinFunction originalBulletTracePassed;

const char *const profileNames[ProfileSlots] = {"target",
                                                "aim",
                                                "movement",
                                                "path",
                                                "native_search",
                                                "native_aim_setup",
                                                "server_frame",
                                                "target_tags",
                                                "target_bullettrace",
                                                "target_sighttrace",
                                                "target_bulletpassed",
                                                "target_other",
                                                "bot_input",
                                                "native_controller",
                                                "native_sight"};

void LogConfiguration(const char *event)
{
    char line[512];
    _snprintf_s(line, sizeof(line), _TRUNCATE,
                "[native bots] %s map=%.48s mod=%.64s profile_requested=%d profile_active=%d path=%d aim=%d "
                "sight_prefilter=%d graph_ready=%d nodes=%u awareness_quality=%d sv_fps=%d sight_order=%d controller=%d\n",
                event, Dvar_GetString("mapname"), Config::GetActiveMod(), Dvar_GetInt("bots_native_profile"),
                profilingEnabled ? 1 : 0, Dvar_GetInt("bots_native_path"), Dvar_GetInt("bots_native_aim"),
                Dvar_GetInt("bots_native_sight_prefilter"), graph.ready ? 1 : 0,
                static_cast<unsigned int>(graph.nodes.size()), Dvar_GetInt("bots_main_awareness_quality"),
                Dvar_GetInt("sv_fps"), Dvar_GetInt("bots_native_sight_order"), Dvar_GetInt("bots_native_controller"));
    // Format locally: pass a single string argument to the Xbox debug transport.
    DbgPrint("%s", line);
}

unsigned __int64 Timestamp()
{
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value);
    return value.QuadPart;
}

void RecordProfile(int slot, unsigned __int64 elapsed)
{
    Profile &profile = profiles[slot];
    profile.total += elapsed;
    if (elapsed > profile.maximum)
        profile.maximum = elapsed;
    ++profile.calls;
}

void FinishProfile(int slot, unsigned __int64 start)
{
    RecordProfile(slot, Timestamp() - start);
}

unsigned __int64 TargetParts()
{
    return profiles[TargetTags].total + profiles[TargetBulletTrace].total + profiles[TargetSightTrace].total +
           profiles[TargetBulletPassed].total;
}

// Hook G_RunFrame itself: SV_ServerThread inlines the SV_RunFrame wrapper and calls this entry directly.
// Retail TU4 0x822731F8 matches debug G_RunFrame and KisakCOD's void G_RunFrame(int levelTime).
void GameFrameHook(int levelTime)
{
    const unsigned int generation = profileGeneration;
    const bool measure = profilingEnabled;
    const unsigned __int64 start = measure ? Timestamp() : 0;
    serverFrameDetour.GetOriginal<void (*)(int)>()(levelTime);
    if (measure && profilingEnabled)
    {
        // A report/reset can run inside the VM during this frame. Do not mix two reporting windows.
        if (generation == profileGeneration)
            FinishProfile(6, start);
        else
            ++skippedBoundaryFrames;
    }
    // These script scopes are synchronous. Discard an unfinished marker after a script error/longjmp.
    for (int i = 0; i < 4; ++i)
        profiles[i].running = false;
}

bool InTargetScope()
{
    return profilingEnabled && profiles[0].running;
}

void GetTagOriginProfiled(scr_entref_t entref)
{
    const bool measure = InTargetScope();
    const unsigned __int64 start = measure ? Timestamp() : 0;
    originalGetTagOrigin(entref);
    if (measure)
        FinishProfile(TargetTags, start);
}

void BulletTraceProfiled()
{
    const bool measure = InTargetScope();
    const unsigned __int64 start = measure ? Timestamp() : 0;
    originalBulletTrace();
    if (measure)
        FinishProfile(TargetBulletTrace, start);
}

void SightTracePassedProfiled()
{
    const bool measure = InTargetScope();
    const unsigned __int64 start = measure ? Timestamp() : 0;
    originalSightTracePassed();
    if (measure)
        FinishProfile(TargetSightTrace, start);
}

void BulletTracePassedProfiled()
{
    const bool measure = InTargetScope();
    const unsigned __int64 start = measure ? Timestamp() : 0;
    originalBulletTracePassed();
    if (measure)
        FinishProfile(TargetBulletPassed, start);
}

void Params(unsigned int count, const char *usage)
{
    if (Scr_GetNumParam() != count)
        Scr_Error(usage);
}

int BotIndex(scr_entref_t entref)
{
    GetPlayerEntity(entref);
    if (entref.entnum >= MAX_CLIENTS || entref.entnum >= svsHeader->maxclients || !svsHeader->clients ||
        svsHeader->clients[entref.entnum].header.netchan.remoteAddress.type != NA_BOT)
        Scr_ObjectError("native bot operation requires a bot client");
    return entref.entnum;
}

void ReadVector(unsigned int param, iw3_bot_native::Vec3 &value)
{
    Scr_GetVector(param, value.v);
    for (int i = 0; i < 3; ++i)
        if (!iw3_bot_native::Finite(value.v[i]) || fabsf(value.v[i]) > 1000000.0f)
            Scr_ParamError(param, "native bot vector must contain finite world coordinates");
}
} // namespace

void Install()
{
    serverFrameDetour = Detour(reinterpret_cast<void *>(0x822731F8), GameFrameHook);
    // Copy through stwu, after the two floating-point spills. Jumping back at +16 uses the detour's
    // -0x30 scratch slot before the frame exists, overwriting the r27 saved by __savegprlr_19.
    serverFrameDetour.Install(20);
    DbgPrint(
        "[native bots] backend loaded; profile_output=DbgPrint; diagnostics=6; frame_hook=G_RunFrame; prologue=20; controller=1\n");
}

BuiltinMethod ProfileMethod(const char *name, BuiltinMethod original)
{
    if (name && original && !_stricmp(name, "gettagorigin") && original != GetTagOriginProfiled)
    {
        originalGetTagOrigin = original;
        return GetTagOriginProfiled;
    }
    return original;
}

BuiltinFunction ProfileFunction(const char *name, BuiltinFunction original)
{
    if (!name || !original)
        return original;
    if (!_stricmp(name, "bullettrace") && original != BulletTraceProfiled)
    {
        originalBulletTrace = original;
        return BulletTraceProfiled;
    }
    if (!_stricmp(name, "sighttracepassed") && original != SightTracePassedProfiled)
    {
        originalSightTracePassed = original;
        return SightTracePassedProfiled;
    }
    if (!_stricmp(name, "bullettracepassed") && original != BulletTracePassedProfiled)
    {
        originalBulletTracePassed = original;
        return BulletTracePassedProfiled;
    }
    return original;
}

unsigned __int64 BeginBotInput()
{
    return profilingEnabled ? Timestamp() : 0;
}

void EndBotInput(unsigned __int64 start)
{
    if (start && profilingEnabled)
        FinishProfile(BotInput, start);
}

void Shutdown()
{
    serverFrameDetour.Remove();
    Reset();
}

void ProfileReset()
{
    Params(0, "botnativeprofilereset()");
    ZeroMemory(profiles, sizeof(profiles));
    profileWindowStart = Timestamp();
    ++profileGeneration;
    skippedBoundaryFrames = 0;
    targetPartsAtStart = 0;
    controller.counters = NativeController::Counters();
    const bool starting = !profilingEnabled;
    profilingEnabled = true;
    if (starting)
        LogConfiguration("profiling enabled; interval=10s");
}

void Reset()
{
    controller.Reset();
    controllerPaused = true;
    controllerDrive = true;
    controllerFrame = -1;
    controllerConfigTime = 0;
    for (int i = 0; i < MAX_CLIENTS; ++i) controllerActors[i] = NativeController::Actor();
    graph.Clear();
    std::vector<int>().swap(resultPath);
    for (int i = 0; i < MAX_CLIENTS; ++i)
        ClearAim(i);
    ZeroMemory(profiles, sizeof(profiles));
    profilingEnabled = false;
    profileWindowStart = 0;
    ++profileGeneration;
    skippedBoundaryFrames = 0;
    targetPartsAtStart = 0;
}

void ClearAim(int clientNum)
{
    aims[clientNum].active = false;
    aimOwners[clientNum] = nullptr;
}

void TickAim(int clientNum, client_t *client)
{
    if (!aims[clientNum].active)
        return;
    gentity_s *entity = client->gentity;
    if (!entity || entity != aimOwners[clientNum] || !entity->client || entity->health <= 0 ||
        entity->client->sess.archiveTime || client->header.state != CS_ACTIVE)
    {
        ClearAim(clientNum);
        return;
    }
    if (aims[clientNum].Tick(static_cast<unsigned int>(svsHeader->time)))
        SetClientViewAngle(entity, aims[clientNum].angle);
}

void NavBegin()
{
    Params(1, "botnavbegin(nodeCount)");
    const int count = Scr_GetInt(0);
    const bool accepted = graph.Begin(count);
    if (count == 0)
        LogConfiguration("script init");
    else if (!accepted)
        LogConfiguration("graph upload rejected; using script paths");
    Scr_AddInt(accepted);
}

void NavNode()
{
    if (Scr_GetNumParam() != 3 && Scr_GetNumParam() != 4)
        Scr_Error("botnavnode(index, origin, extraCost [, traversalFlags])");
    iw3_bot_native::Vec3 origin;
    ReadVector(1, origin);
    const int index = Scr_GetInt(0);
    const bool accepted = graph.SetNode(index, origin, Scr_GetInt(2));
    if (accepted && Scr_GetNumParam() == 4)
        graph.nodes[index].traversal = Scr_GetInt(3) & 15;
    Scr_AddInt(accepted);
}

void NavLink()
{
    Params(2, "botnavlink(from, to)");
    const int from = Scr_GetInt(0), to = Scr_GetInt(1);
    const bool accepted = graph.Link(from, to);
    if (!accepted)
    {
        char line[160];
        _snprintf_s(line, sizeof(line), _TRUNCATE,
                    "[native bots] graph link rejected from=%d to=%d; using script paths\n", from, to);
        DbgPrint("%s", line);
    }
    Scr_AddInt(accepted);
}

void NavEnd()
{
    Params(0, "botnavend()");
    const bool accepted = graph.Finish();
    LogConfiguration(accepted ? "graph ready" : "graph incomplete; using script paths");
    Scr_AddInt(accepted);
}

void NavNearest()
{
    Params(1, "botnavnearest(origin)");
    iw3_bot_native::Vec3 origin;
    ReadVector(0, origin);
    const int nearest = graph.ready ? graph.Nearest(origin) : -1;
    if (nearest >= 0)
        Scr_AddInt(nearest);
    else
        Scr_AddUndefined();
}

void NavSearch()
{
    Params(4, "botnavsearch(startIndex, goalIndex, teamIndex, greedy)");
    const unsigned __int64 start = profilingEnabled ? Timestamp() : 0;
    const bool success = graph.Search(Scr_GetInt(0), Scr_GetInt(1), Scr_GetInt(2), Scr_GetInt(3) != 0, resultPath);
    if (profilingEnabled)
        FinishProfile(4, start);
    if (!success)
    {
        Scr_AddUndefined();
        return;
    }
    Scr_MakeArray();
    for (size_t i = 0; i < resultPath.size(); ++i)
    {
        Scr_AddInt(resultPath[i]);
        Scr_AddArray();
    }
}

void NavUsage()
{
    Params(3, "botnavusage(index, teamIndex, delta)");
    Scr_AddInt(graph.Usage(Scr_GetInt(0), Scr_GetInt(1), Scr_GetInt(2)));
}

void AimAt(scr_entref_t entref)
{
    const int index = BotIndex(entref);
    Params(5, "<bot> botnativeaim(eye, position, seconds, targetVelocity, predict)");
    iw3_bot_native::Vec3 eye, position, velocity;
    ReadVector(0, eye);
    ReadVector(1, position);
    const float seconds = Scr_GetFloat(2);
    ReadVector(3, velocity);
    const bool predict = Scr_GetInt(4) != 0;
    if (!iw3_bot_native::Finite(seconds) || seconds < 0 || seconds > 30)
        Scr_ParamError(2, "native bot aim duration must be between 0 and 30 seconds");

    gentity_s *entity = &g_entities[index];
    if (!entity->client || entity->health <= 0 || entity->client->sess.archiveTime)
    {
        ClearAim(index);
        return;
    }
    const unsigned __int64 start = profilingEnabled ? Timestamp() : 0;
    iw3_bot_native::Vec3 ownVelocity;
    memcpy(ownVelocity.v, entity->client->ps.velocity, sizeof(ownVelocity.v));
    aims[index].Start(eye, position, entity->client->ps.viewangles, ownVelocity, velocity, seconds, predict,
                      static_cast<unsigned int>(svsHeader->time));
    aimOwners[index] = entity;
    // bot_lookat also applies its first interpolation step before its first wait.
    TickAim(index, &svsHeader->clients[index]);
    if (profilingEnabled)
        FinishProfile(5, start);
}

void AimStop(scr_entref_t entref)
{
    Params(0, "<bot> botnativeaimstop()");
    ClearAim(BotIndex(entref));
}

void ProfileBegin()
{
    Params(1, "botnativeprofilebegin(slot)");
    const int slot = Scr_GetInt(0);
    if (slot < 0 || slot > 3)
        Scr_ParamError(0, "profile slot must be in [0, 3]");
    profiles[slot].start.QuadPart = Timestamp();
    profiles[slot].running = true;
    if (slot == 0)
        targetPartsAtStart = TargetParts();
}

void ProfileEnd()
{
    Params(1, "botnativeprofileend(slot)");
    const int slot = Scr_GetInt(0);
    if (slot < 0 || slot > 3)
        Scr_ParamError(0, "profile slot must be in [0, 3]");
    if (profiles[slot].running)
    {
        const unsigned __int64 elapsed = Timestamp() - profiles[slot].start.QuadPart;
        RecordProfile(slot, elapsed);
        if (slot == 0)
        {
            const unsigned __int64 parts = TargetParts() - targetPartsAtStart;
            RecordProfile(TargetOther, elapsed > parts ? elapsed - parts : 0);
        }
        profiles[slot].running = false;
    }
}

void ProfileStats()
{
    Params(0, "botnativeprofilestats()");
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    const double micros = frequency.QuadPart > 0 ? 1000000.0 / frequency.QuadPart : 0;
    LogConfiguration("profile report");
    char line[256];
    _snprintf_s(line, sizeof(line), _TRUNCATE,
                "[native bots] window_ms=%.1f frame_boundary_skips=%u tag_hook=%d bullet_hook=%d sight_hook=%d "
                "bulletpassed_hook=%d\n",
                profilingEnabled ? (Timestamp() - profileWindowStart) * micros / 1000.0 : 0.0, skippedBoundaryFrames,
                originalGetTagOrigin ? 1 : 0, originalBulletTrace ? 1 : 0, originalSightTracePassed ? 1 : 0,
                originalBulletTracePassed ? 1 : 0);
    DbgPrint("%s", line);
    const NativeController::Counters &c = controller.counters;
    int active = 0;
    for (int i = 0; i < MAX_CLIENTS; ++i)
        if (controller.bots[i].enabled && controllerActors[i].alive) ++active;
    _snprintf_s(line, sizeof(line), _TRUNCATE,
                "[native bots] controller active=%d paused=%d drive=%d frames=%u sight=%u paths=%u acquired=%u "
                "stuck=%u no_route=%u sight_budget=%d path_budget=%d reaction_ms=%d capacity=%d allocated=%d awareness=unused\n",
                active, controllerPaused ? 1 : 0, controllerDrive ? 1 : 0, c.frames, c.sight, c.paths, c.acquisitions,
                c.stuck, c.noRoute, controller.settings.sightBudget, controller.settings.pathBudget,
                controller.settings.reactionMs, NativeController::Clients, svsHeader->maxclients);
    DbgPrint("%s", line);
    Scr_MakeArray();
    for (int i = 0; i < ProfileSlots; ++i)
    {
        const double totalMicros = profiles[i].total * micros;
        const double averageMicros = profiles[i].calls ? totalMicros / profiles[i].calls : 0;
        _snprintf_s(line, sizeof(line), _TRUNCATE, "[native bots] %s calls=%u total_us=%.1f avg_us=%.1f max_us=%.1f\n",
                    profileNames[i], profiles[i].calls, totalMicros, averageMicros, profiles[i].maximum * micros);
        // This is deliberately independent of the game's println/channel filters and Debug-only console hook.
        // Keep the returned array compatible with the first experimental mod's reporting script.
        DbgPrint("%s", line);
        Scr_AddInt(static_cast<int>(profiles[i].calls));
        Scr_AddArray();
        Scr_AddFloat(static_cast<float>(profiles[i].total * micros));
        Scr_AddArray();
        Scr_AddFloat(static_cast<float>(profiles[i].maximum * micros));
        Scr_AddArray();
    }
}
#include "bot_native_controller.inl"
} // namespace bot_native
} // namespace mp
} // namespace iw3
