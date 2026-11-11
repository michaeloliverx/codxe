// Compile with -DNDEBUG: exercise Release's logging path, not the Debug-only console hook.
#include "../../../src/game/iw3/mp/components/experimental/bot_native.cpp"
#include <cstdlib>
#include <iostream>

#define CHECK(condition)                                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(condition))                                                                                              \
        {                                                                                                              \
            std::cerr << "FAIL line " << __LINE__ << ": " << #condition << '\n';                                       \
            std::exit(1);                                                                                              \
        }                                                                                                              \
    } while (0)

bool Logged(const std::string &text)
{
    for (const std::string &message : profile_test::messages)
        if (message.find(text) != std::string::npos)
            return true;
    return false;
}

void FakeTag(iw3::mp::scr_entref_t entref)
{
    CHECK(entref.entnum == 3 && entref.classnum == 0);
    CHECK(iw3::mp::Scr_GetNumParam() == 1);
    profile_test::ticks += 100;
    iw3::mp::Scr_AddInt(37);
}

void FakeBullet()
{
    profile_test::ticks += 50;
    iw3::mp::Scr_AddInt(43);
}

void FakeSight()
{
    profile_test::ticks += 70;
    iw3::mp::Scr_AddInt(1);
}

void FakePassed()
{
    profile_test::ticks += 30;
    iw3::mp::Scr_AddInt(0);
}

int nativeQueries, fxQueries;
bool blockNative;
double fxVisibility = 1;
void FakeNativeSight(int *hit, const float *, const float *mins, const float *maxs, const float *, int self,
                     int other, int mask)
{
    CHECK(*hit == 0 && mask == 0x801803 && self != other);
    for (int k = 0; k < 3; ++k) CHECK(mins[k] == 0 && maxs[k] == 0);
    ++nativeQueries;
    profile_test::ticks += 12;
    *hit = blockNative ? 1 : 0;
}
double FakeNativeFx(int client, const float *, const float *)
{
    CHECK(client == 0);
    ++fxQueries;
    profile_test::ticks += 3;
    return fxVisibility;
}

int main()
{
    using namespace iw3::mp::bot_native;
    using namespace profile_test;
#ifndef NDEBUG
    std::cerr << "This regression test must be built with NDEBUG\n";
    return 2;
#endif
    Install();
    CHECK(Logged("backend loaded; profile_output=DbgPrint; diagnostics=6; frame_hook=G_RunFrame; prologue=20"));
    Reset();
    numParams = 0;
    ticks = 100;
    ProfileReset();
    CHECK(Logged("profiling enabled; interval=10s"));
    CHECK(Logged("map=mp_crash mod=bot_warfare_native profile_requested=1 profile_active=1"));
    numParams = 1;
    params[0] = 0;
    NavBegin();
    CHECK(Logged("script init"));
    params[0] = 2;
    NavBegin();
    CHECK(returned.back() == 1);
    numParams = 3;
    params[0] = 0;
    params[2] = 0;
    NavNode();
    params[0] = 1;
    NavNode();
    numParams = 0;
    NavEnd();
    CHECK(Logged("graph ready"));
    CHECK(Logged("graph_ready=1 nodes=2"));

    const auto tag = ProfileMethod("GetTagOrigin", FakeTag);
    const auto bullet = ProfileFunction("bullettrace", FakeBullet);
    const auto sight = ProfileFunction("sighttracepassed", FakeSight);
    const auto passed = ProfileFunction("bullettracepassed", FakePassed);
    CHECK(ProfileMethod("gettagorigin", tag) == tag);
    CHECK(ProfileFunction("bullettrace", bullet) == bullet);
    CHECK(ProfileFunction("other", FakeBullet) == FakeBullet);
    CHECK(ProfileFunction(nullptr, FakeBullet) == FakeBullet);
    CHECK(ProfileMethod("gettagorigin", nullptr) == nullptr);
    numParams = 1;
    tag({3, 0}); // Outside target: exact original call/result, no target timing sample.
    CHECK(returned.back() == 37);

    // Drive actual public scope callbacks and the outer server-frame hook through SDK/VM stubs.
    numParams = 1;
    params[0] = 0;
    ticks = 200;
    ProfileBegin();
    tag({3, 0});
    CHECK(returned.back() == 37);
    bullet();
    CHECK(returned.back() == 43);
    sight();
    CHECK(returned.back() == 1);
    passed();
    CHECK(returned.back() == 0);
    ticks += 250;
    ProfileEnd();
    GameFrameHook(1234);
    CHECK(lastFrameTime == 1234);
    const auto inputStart = BeginBotInput();
    ticks += 80;
    EndBotInput(inputStart);
    ticks = 10000100;
    numParams = 0;
    messages.clear();
    ProfileStats();
    CHECK(Logged("profile report"));
    CHECK(Logged("window_ms=10000.0"));
    CHECK(Logged("target calls=1 total_us=500.0 avg_us=500.0 max_us=500.0"));
    CHECK(Logged("server_frame calls=1 total_us=40.0 avg_us=40.0 max_us=40.0"));
    CHECK(Logged("aim calls=0 total_us=0.0 avg_us=0.0 max_us=0.0"));
    CHECK(Logged("target_tags calls=1 total_us=100.0 avg_us=100.0 max_us=100.0"));
    CHECK(Logged("target_bullettrace calls=1 total_us=50.0"));
    CHECK(Logged("target_sighttrace calls=1 total_us=70.0"));
    CHECK(Logged("target_bulletpassed calls=1 total_us=30.0"));
    CHECK(Logged("target_other calls=1 total_us=250.0"));
    CHECK(Logged("bot_input calls=1 total_us=80.0"));
    CHECK(Logged("tag_hook=1 bullet_hook=1 sight_hook=1 bulletpassed_hook=1"));
    CHECK(Logged("awareness_quality=2 sv_fps=20"));
    CHECK(messages.size() == 18); // Configuration, window, controller counters, and fifteen scopes.
    CHECK(returned.size() == 45 && returned[0] == 1 && returned[1] == 500 && returned[2] == 500);
    for (const std::string &message : messages)
        CHECK(message.find("[native bots]") == 0 && message.back() == '\n');
    for (const std::string &message : messages)
        std::cout << message;

    // A ten-second reset starts a fresh window without repeating the startup marker.
    messages.clear();
    ProfileReset();
    CHECK(messages.empty());
    ProfileStats();
    CHECK(returned.size() == 45 && returned[0] == 0 && returned[1] == 0);

    // The report/reset runs inside G_RunFrame. A boundary frame must not contaminate the next window.
    messages.clear();
    duringFrame = ProfileReset;
    GameFrameHook(5678);
    duringFrame = nullptr;
    ProfileStats();
    CHECK(Logged("frame_boundary_skips=1"));
    CHECK(Logged("server_frame calls=0 total_us=0.0"));

    // Reporting graph failure is also Release-visible, and never claims native navigation is ready.
    messages.clear();
    numParams = 1;
    params[0] = 1;
    NavBegin();
    numParams = 2;
    params[0] = 0;
    params[1] = -177;
    NavLink();
    CHECK(Logged("graph link rejected from=0 to=-177; using script paths"));
    CHECK(returned.back() == 0);

    // Exercise the production driver, direct trace ABI, paused/dead/frozen input and one lobby sweep per time.
    using namespace iw3::mp;
    Reset();
    static gclient_s live[MAX_CLIENTS] = {};
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        client_t &cl = svsHeader->clients[i];
        cl.header.state = CS_ACTIVE;
        cl.header.netchan.remoteAddress.type = NA_BOT;
        cl.gentity = &g_entities[i];
        g_entities[i].client = &live[i];
        g_entities[i].health = 100;
        live[i].sess.cs.team = i % 2 + 1;
        live[i].ps.origin[0] = static_cast<float>(i * 20);
        live[i].ps.viewHeightCurrent = 60;
        for (int k = 0; k < 3; ++k) live[i].sess.cmd.angles[k] = 123 + k;
        numParams = 1; params[0] = 1;
        if (i < 17) Control(scr_entref_t{static_cast<unsigned short>(i), 0});
    }
    nativeSightTrace = FakeNativeSight;
    nativeFxVisibility = FakeNativeFx;
    int fxClient = 0;
    nativeFxClient = &fxClient;
    usercmd_s input = {};
    input.buttons = KEY_MASK_FIRE; input.forwardmove = 127;
    CHECK(TickController(0, &svsHeader->clients[0], input));
    CHECK(input.buttons == 0 && input.forwardmove == 0 && nativeQueries == 0);
    CHECK(!TickController(17, &svsHeader->clients[17], input));
    numParams = 1; params[0] = 0; Pause();
    const size_t vmResults = returned.size();
    svsHeader->time = 1000;
    for (int i = 0; i < 17; ++i) CHECK(TickController(i, &svsHeader->clients[i], input));
    CHECK(controller.counters.frames == 1 && nativeQueries == 6 && fxQueries == 6);
    CHECK(returned.size() == vmResults); // No VM result/parameter operations during the native tick.
    CHECK(input.angles[0] == 123 && input.angles[1] == 124 && input.angles[2] == 125);
    live[0].bFrozen = 1;
    input.buttons = KEY_MASK_FIRE;
    CHECK(TickController(0, &svsHeader->clients[0], input) && input.buttons == 0);
    live[0].bFrozen = 0;
    live[0].sess.sessionState = SESS_STATE_DEAD;
    g_entities[0].health = 0;
    svsHeader->time = 1250;
    CHECK(TickController(0, &svsHeader->clients[0], input) && input.buttons == KEY_MASK_FIRE);
    live[0].sess.archiveTime = 5;
    CHECK(TickController(0, &svsHeader->clients[0], input) && input.buttons == 0);
    ControllerWorld nativeWorld;
    iw3_bot_native::Vec3 point = {{0, 0, 0}};
    blockNative = true;
    const int oldFx = fxQueries;
    CHECK(!nativeWorld.Visible(0, 1, point, point) && fxQueries == oldFx);
    blockNative = false; fxVisibility = 0.1;
    CHECK(!nativeWorld.Visible(0, 1, point, point));
    fxVisibility = 0.2;
    CHECK(nativeWorld.Visible(0, 1, point, point));
    numParams = 1; params[0] = 0; Control(scr_entref_t{0, 0});
    CHECK(!TickController(0, &svsHeader->clients[0], input));

    // Production used MAX_CLIENTS=24 while the original core had only 18 entries. Exercise the actual limit.
    CHECK(static_cast<int>(NativeController::Clients) == MAX_CLIENTS && MAX_CLIENTS == 24);
    controller.Reset();
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        live[i].sess.sessionState = SESS_STATE_PLAYING;
        live[i].sess.archiveTime = 0;
        g_entities[i].health = 100;
    }
    params[0] = 1;
    for (int i = 1; i < MAX_CLIENTS; ++i)
    {
        svsHeader->time = 2000 + i * 50;
        Control(scr_entref_t{static_cast<unsigned short>(i), 0});
        CHECK(TickController(i, &svsHeader->clients[i], input));
        CHECK(controller.settings.sightBudget == 6 && controller.settings.reactionMs == 250);
    }
    CHECK(controller.bots[23].enabled && controller.counters.frames == 23);
    CHECK(!TickController(-1, nullptr, input) && !TickController(MAX_CLIENTS, nullptr, input));
    // Slots beyond the server's allocation must not be read even if the compiled capacity is higher.
    svsHeader->maxclients = 18;
    for (int i = 18; i < MAX_CLIENTS; ++i) svsHeader->clients[i].gentity = reinterpret_cast<gentity_s *>(1);
    CHECK(!ReadActor(18).alive && !ReadActor(23).alive && !ReadActor(-1).alive);
    CHECK(!TickController(18, &svsHeader->clients[18], input));
    bool rejected = false;
    try { Control(scr_entref_t{18, 0}); } catch (const std::runtime_error &) { rejected = true; }
    CHECK(rejected);
    Shutdown();
    CHECK(BeginBotInput() == 0);
    EndBotInput(0);

    // Even when profiling was not requested, the script-init marker identifies the missing flag.
    messages.clear();
    profileRequested = 0;
    numParams = 1;
    params[0] = 0;
    NavBegin();
    CHECK(Logged("profile_requested=0 profile_active=0"));
    std::cout << "PASS: G_RunFrame arguments, target builtin passthrough/scope filtering, residual timing, bot input, "
                 "window boundaries, Release logging, legacy VM array prefix, native lobby driver, trace/FX gate, "
                 "pause, freeze, death/respawn, 24-slot addition and allocated-client bounds\n";
}
