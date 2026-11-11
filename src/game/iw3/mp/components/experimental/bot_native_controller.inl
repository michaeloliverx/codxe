// Included inside bot_native's namespace to share the map graph and profiler without extra engine bindings.
namespace
{
// Retail TU4 Scr_SightTracePassed at 0x8225C318 calls these exact entries (and this FX client index).
// SV_SightTrace is a point-only implementation in this retail build; pass zero extents as stock does.
typedef void (*NativeSightTrace_t)(int *, const float *, const float *, const float *, const float *, int, int, int);
NativeSightTrace_t nativeSightTrace = reinterpret_cast<NativeSightTrace_t>(0x82355488);
typedef double (*NativeFxVisibility_t)(int, const float *, const float *);
NativeFxVisibility_t nativeFxVisibility = reinterpret_cast<NativeFxVisibility_t>(0x8228F868);
int *nativeFxClient = reinterpret_cast<int *>(0x82725F64);

struct ControllerWorld
{
    bool Visible(int self, int other, const iw3_bot_native::Vec3 &start, const iw3_bot_native::Vec3 &end)
    {
        const unsigned __int64 before = profilingEnabled ? Timestamp() : 0;
        const float zero[3] = {0, 0, 0};
        int hit = 0; // The engine uses this as an input hint as well as an output.
        nativeSightTrace(&hit, start.v, zero, zero, end.v, self, other, 0x801803);
        // Preserve the stock Xenon sight builtin's FX visibility gate, without GSC smoke-list iteration.
        const bool visible = !hit && nativeFxVisibility(*nativeFxClient, start.v, end.v) >= 0.2;
        if (profilingEnabled) FinishProfile(NativeSight, before);
        return visible;
    }
};

int BoundedSetting(const char *name, int minimum, int maximum)
{
    return (std::max)(minimum, (std::min)(maximum, Dvar_GetInt(name)));
}

void ReadControllerSettings(int now)
{
    if (now < controllerConfigTime) return;
    controllerConfigTime = now + 500;
    controllerDrive = Dvar_GetInt("bots_native_drive") != 0;
    controller.settings.sightBudget = BoundedSetting("bots_native_sight_budget", 1, 18);
    controller.settings.pathBudget = BoundedSetting("bots_native_path_budget", 0, 2);
    controller.settings.reactionMs = BoundedSetting("bots_native_reaction_ms", 0, 2000);
    controller.settings.range = static_cast<float>(BoundedSetting("bots_native_range", 256, 12000));
    const float halfFov = BoundedSetting("bots_native_fov", 30, 360) * 0.008726646259971648f;
    controller.settings.fovCos = cosf(halfFov);
    controller.settings.turnRate = static_cast<float>(BoundedSetting("bots_native_turn_rate", 30, 1080));
}

NativeController::Actor ReadActor(int index)
{
    NativeController::Actor actor;
    if (!NativeController::ValidClient(index) || index >= svsHeader->maxclients || !svsHeader->clients) return actor;
    client_t *cl = &svsHeader->clients[index];
    gentity_s *entity = cl->gentity;
    if (cl->header.state != CS_ACTIVE || !entity || !entity->client || entity->health <= 0) return actor;
    const gclient_s &client = *entity->client;
    actor.team = client.sess.cs.team;
    actor.alive = client.sess.sessionState == SESS_STATE_PLAYING && !client.sess.archiveTime &&
                  client.ps.pm_type == 0 && actor.team != TEAM_SPECTATOR;
    if (!actor.alive) return actor;
    memcpy(actor.origin.v, client.ps.origin, sizeof(actor.origin.v));
    actor.eye = actor.origin;
    actor.eye.v[2] += client.ps.viewHeightCurrent;
    memcpy(actor.angles, client.ps.viewangles, sizeof(actor.angles));
    return actor;
}
} // namespace

void Control(scr_entref_t entref)
{
    Params(1, "<bot> botnativecontrol(enabled)");
    const int index = BotIndex(entref);
    ClearAim(index);
    if (Scr_GetInt(0)) controller.Start(index, svsHeader->time);
    else controller.Stop(index);
}

void Pause()
{
    Params(1, "botnativepause(paused)");
    controllerPaused = Scr_GetInt(0) != 0;
}

bool TickController(int index, client_t *client, usercmd_s &cmd)
{
    if (!NativeController::ValidClient(index) || index >= svsHeader->maxclients || !client ||
        !controller.bots[index].enabled) return false;
    // An enabled native bot owns its commands even when dead, paused, or awaiting a route.
    cmd.buttons = 0;
    cmd.forwardmove = cmd.rightmove = 0;
    const gclient_s *inputClient = client->gentity ? client->gentity->client : nullptr;
    // Preserve the input-angle baseline while paused/dead/frozen as well as while actively turning.
    if (inputClient) memcpy(cmd.angles, inputClient->sess.cmd.angles, sizeof(cmd.angles));
    const int now = svsHeader->time;
    ReadControllerSettings(now);
    if (controllerPaused || !controllerDrive) return true;
    const unsigned __int64 before = profilingEnabled ? Timestamp() : 0;
    if (controllerFrame != now)
    {
        controllerFrame = now;
        for (int i = 0; i < MAX_CLIENTS; ++i) controllerActors[i] = ReadActor(i);
        ControllerWorld world;
        controller.Frame(now, controllerActors, graph, world);
    }
    const NativeController::Actor actor = ReadActor(index);
    if (inputClient && inputClient->sess.sessionState == SESS_STATE_DEAD && !inputClient->sess.archiveTime &&
        (now / 250) % 2)
        cmd.buttons |= KEY_MASK_FIRE; // Request respawn on servers without forced respawning.
    if (actor.alive && !client->gentity->client->bFrozen && !(client->gentity->client->ps.pm_flags & (1 << 11)))
    {
        NativeController::Output out = controller.Input(index, now, actor, graph);
        // SetClientViewAngle derives delta_angles from sess.cmd, matching cmd.angles above.
        SetClientViewAngle(client->gentity, out.angles);
        cmd.forwardmove = static_cast<char>(out.forward);
        cmd.rightmove = static_cast<char>(out.right);
        if (out.fire) cmd.buttons |= KEY_MASK_FIRE;
        if (out.ads) cmd.buttons |= KEY_MASK_ADS | KEY_MASK_ADS_MODE;
        if (out.reload) cmd.buttons |= KEY_MASK_RELOAD;
        if (out.sprint) cmd.buttons |= KEY_MASK_SPRINT;
        if (out.jump) cmd.buttons |= KEY_MASK_JUMP;
        if (out.crouch) cmd.buttons |= KEY_MASK_CROUCH;
        if (out.prone) cmd.buttons |= KEY_MASK_PRONE;
        // Otherwise standing is requested through the command's absence of crouch/prone bits.
    }
    if (profilingEnabled) FinishProfile(NativeControllerProfile, before);
    return true;
}
