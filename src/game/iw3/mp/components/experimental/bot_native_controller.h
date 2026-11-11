#pragma once

#include "bot_native_core.h"

// Performance experiment: fixed lobby state, bounded sight/path work, no VM callbacks in the tick.
namespace iw3_bot_native
{
template <int ClientCapacity> class ControllerFor
{
  public:
    static_assert(ClientCapacity > 0, "native controller requires a positive client capacity");
    enum { Clients = ClientCapacity };
    struct Actor
    {
        bool alive;
        int team;
        Vec3 origin, eye;
        float angles[3];
        Actor() : alive(false), team(0)
        {
            for (int k = 0; k < 3; ++k) origin.v[k] = eye.v[k] = angles[k] = 0;
        }
    };
    struct Settings
    {
        int sightBudget, pathBudget, reactionMs;
        float range, fovCos, turnRate;
        Settings() : sightBudget(6), pathBudget(1), reactionMs(250), range(4000), fovCos(0.0f), turnRate(300) { }
    };
    struct Output
    {
        float angles[3];
        int forward, right;
        bool fire, ads, reload, sprint, jump, crouch, prone;
        Output() : forward(0), right(0), fire(false), ads(false), reload(false), sprint(false), jump(false),
                   crouch(false), prone(false) { angles[0] = angles[1] = angles[2] = 0; }
    };
    struct Bot
    {
        bool enabled, visible, moving;
        int target, candidateCursor, targetSince, lastSeen, nextSense, lastInput, nextPlan, routeIndex;
        int progressTime, stuckSince, nextJump, reloadAt;
        unsigned int random;
        Vec3 lastKnown, progressOrigin;
        std::vector<int> route;
        Bot() : enabled(false), visible(false), moving(false), target(-1), candidateCursor(0), targetSince(0),
                lastSeen(-10000), nextSense(0), lastInput(0), nextPlan(0), routeIndex(-1), progressTime(0),
                stuckSince(0), nextJump(0), reloadAt(0), random(1)
        {
            for (int k = 0; k < 3; ++k) lastKnown.v[k] = progressOrigin.v[k] = 0;
        }
    };
    struct Counters
    {
        unsigned int frames, sight, paths, acquisitions, stuck, noRoute;
        Counters() : frames(0), sight(0), paths(0), acquisitions(0), stuck(0), noRoute(0) { }
    };
    Bot bots[Clients];
    Settings settings;
    Counters counters;

    ControllerFor() : sightCursor(0), pathCursor(0) { }

    static bool ValidClient(int index) { return index >= 0 && index < Clients; }

    bool Start(int index, int now)
    {
        if (!ValidClient(index)) return false;
        Stop(index);
        Bot &b = bots[index];
        b.enabled = true;
        b.random = 0x9e3779b9u ^ (static_cast<unsigned int>(index + 1) * 1664525u) ^ static_cast<unsigned int>(now);
        b.lastInput = now;
        b.nextSense = now;
        b.reloadAt = now + 6000;
        return true;
    }

    bool Stop(int index)
    {
        if (!ValidClient(index)) return false;
        bots[index] = Bot();
        return true;
    }
    void Reset()
    {
        for (int i = 0; i < Clients; ++i) Stop(i);
        sightCursor = pathCursor = 0;
        counters = Counters();
    }

    static bool Enemy(const Actor &self, const Actor &other)
    {
        return other.alive && (self.team == 0 || other.team != self.team);
    }

    // World supplies one direct server sight query, with no script calls or bone/tag work.
    template <class World> void Frame(int now, const Actor *actors, Graph &graph, World &world)
    {
        ++counters.frames;
        for (int i = 0; i < Clients; ++i)
        {
            Bot &b = bots[i];
            if (!b.enabled || !actors[i].alive) continue;
            if (b.target >= 0 && (!Enemy(actors[i], actors[b.target]) || now - b.lastSeen > 2000))
            {
                b.target = -1;
                b.visible = false;
            }
        }
        int queries = 0;
        // Visit each slot at most once per frame. Advance even when it has no eligible candidate.
        for (int visited = 0; visited < Clients && queries < settings.sightBudget; ++visited)
        {
            const int index = sightCursor;
            sightCursor = (sightCursor + 1) % Clients;
            Bot &b = bots[index];
            if (!b.enabled || !actors[index].alive || now < b.nextSense) continue;
            b.nextSense = now + 100;
            int candidate = -1;
            // Revalidate the current target first, but let a blocked target yield to the search next time.
            if (b.target >= 0 && b.visible) candidate = b.target;
            else
            {
                // Rotate cheap candidate tests to prevent a nearer enemy behind cover starving others.
                for (int n = 0; n < Clients; ++n)
                {
                    const int other = b.candidateCursor;
                    b.candidateCursor = (b.candidateCursor + 1) % Clients;
                    if (other != index && Enemy(actors[index], actors[other]) && InCone(actors[index], actors[other]))
                    { candidate = other; break; }
                }
            }
            if (candidate < 0) continue;
            ++queries;
            ++counters.sight;
            const bool seen = world.Visible(index, candidate, actors[index].eye, actors[candidate].eye);
            if (candidate == b.target) b.visible = seen;
            if (seen)
            {
                if (candidate != b.target)
                {
                    b.target = candidate;
                    b.targetSince = now;
                    ++counters.acquisitions;
                }
                b.visible = true;
                b.lastSeen = now;
                b.lastKnown = actors[candidate].eye;
            }
        }
        int plans = 0;
        for (int visited = 0; visited < Clients && plans < settings.pathBudget; ++visited)
        {
            const int index = pathCursor;
            pathCursor = (pathCursor + 1) % Clients;
            Bot &b = bots[index];
            if (!b.enabled || !actors[index].alive || now < b.nextPlan || !graph.ready) continue;
            // Keep a useful route; only chase positions actually observed by a sight query.
            if (b.routeIndex >= 0 && b.target < 0 && !b.stuckSince) continue;
            ++plans;
            ++counters.paths;
            const int start = graph.Nearest(actors[index].origin);
            int goal;
            if (b.target >= 0 && now - b.lastSeen <= 2000)
                goal = graph.Nearest(b.lastKnown);
            else
                goal = static_cast<int>(Random(b) % graph.nodes.size());
            b.route.clear();
            if (graph.Search(start, goal, actors[index].team == 1 ? 1 : 0, true, b.route))
                b.routeIndex = static_cast<int>(b.route.size()) - 1;
            else
            {
                b.routeIndex = -1;
                ++counters.noRoute;
            }
            b.nextPlan = now + (b.target >= 0 ? 1000 : 1500);
            b.stuckSince = 0;
        }
    }

    Output Input(int index, int now, const Actor &actor, const Graph &graph)
    {
        Output out;
        for (int k = 0; k < 3; ++k) out.angles[k] = actor.angles[k];
        if (!ValidClient(index)) return out;
        Bot &b = bots[index];
        if (!b.enabled || !actor.alive) return out;
        int elapsed = now - b.lastInput;
        if (elapsed < 1) elapsed = 1;
        if (elapsed > 100) elapsed = 100;
        b.lastInput = now;
        const bool recent = b.target >= 0 && b.visible && now - b.lastSeen <= 250;
        Vec3 destination = actor.origin;
        bool move = false;
        int traversal = 0;
        while (graph.ready && b.routeIndex >= 0)
        {
            const int node = b.route[b.routeIndex];
            if (!graph.Valid(node)) { b.routeIndex = -1; break; }
            destination = graph.nodes[node].origin;
            if (DistanceSquared(actor.origin, destination) > 36 * 36) { move = true; traversal = graph.nodes[node].traversal; break; }
            --b.routeIndex;
        }
        Vec3 look = destination;
        look.v[2] += actor.eye.v[2] - actor.origin.v[2];
        if (b.target >= 0 && now - b.lastSeen <= 2000) look = b.lastKnown;
        // Eye visibility uses one high point; aim slightly below it toward the upper torso.
        if (recent) look.v[2] -= 10;
        const float dx = look.v[0] - actor.eye.v[0], dy = look.v[1] - actor.eye.v[1], dz = look.v[2] - actor.eye.v[2];
        const float radians = 0.017453292519943295f;
        float yawError = 0, pitchError = 0;
        if (move || b.target >= 0)
        {
            yawError = Angle180(atan2f(dy, dx) / radians - actor.angles[1]);
            pitchError = Angle180(-atan2f(dz, sqrtf(dx * dx + dy * dy)) / radians - actor.angles[0]);
            const float step = settings.turnRate * elapsed / 1000.0f;
            out.angles[0] = Angle180(actor.angles[0] + Clamp(pitchError, -step, step));
            out.angles[1] = Angle180(actor.angles[1] + Clamp(yawError, -step, step));
            out.angles[2] = 0;
        }
        if (recent)
        {
            out.ads = true;
            // One released tick in eight lets semi-auto weapons repeat while retaining most automatic fire.
            out.fire = now - b.targetSince >= settings.reactionMs && fabsf(yawError) < 6 && fabsf(pitchError) < 6 &&
                       ((now / 50 + index) % 8 != 0);
            const float distance = DistanceSquared(actor.eye, b.lastKnown);
            if (distance < 220 * 220) move = false;
            out.right = ((now / 1800 + index) % 2) ? 60 : -60;
        }
        if (move)
        {
            const float x = destination.v[0] - actor.origin.v[0], y = destination.v[1] - actor.origin.v[1];
            const float angle = out.angles[1] * radians;
            float forward = x * cosf(angle) + y * sinf(angle);
            float right = x * sinf(angle) - y * cosf(angle);
            const float scale = (std::max)(fabsf(forward), fabsf(right));
            if (scale > 0)
            {
                out.forward = static_cast<int>(127 * forward / scale);
                out.right = static_cast<int>(127 * right / scale);
            }
            out.sprint = !recent && fabsf(yawError) < 35 && DistanceSquared(destination, actor.origin) > 180 * 180;
            out.crouch = (traversal & 2) != 0;
            out.prone = (traversal & 4) != 0;
            if (((traversal & (1 | 8)) || destination.v[2] - actor.origin.v[2] > 24 || b.stuckSince) && now >= b.nextJump)
            { out.jump = true; b.nextJump = now + 1000; }
            if (now - b.progressTime >= 750)
            {
                if (b.moving && DistanceSquared(actor.origin, b.progressOrigin) < 12 * 12)
                {
                    if (!b.stuckSince) { b.stuckSince = now; ++counters.stuck; }
                    if (now - b.stuckSince >= 1500)
                    {
                        b.routeIndex = -1;
                        b.target = -1;
                        b.visible = false;
                        b.nextPlan = now;
                    }
                }
                else b.stuckSince = 0;
                b.progressOrigin = actor.origin;
                b.progressTime = now;
            }
        }
        b.moving = move;
        // Empty magazines also use the engine's automatic reload. Top up periodically outside combat.
        if (!recent && now >= b.reloadAt) { out.reload = true; b.reloadAt = now + 6000; }
        return out;
    }

  private:
    int sightCursor, pathCursor;
    static float Clamp(float value, float low, float high) { return (std::max)(low, (std::min)(high, value)); }
    static unsigned int Random(Bot &b) { b.random = b.random * 1664525u + 1013904223u; return b.random; }
    bool InCone(const Actor &self, const Actor &other) const
    {
        const float x = other.eye.v[0] - self.eye.v[0], y = other.eye.v[1] - self.eye.v[1];
        const float distance = DistanceSquared(self.eye, other.eye);
        if (distance > settings.range * settings.range) return false;
        if (distance < 192 * 192) return true;
        const float angle = self.angles[1] * 0.017453292519943295f;
        return x * cosf(angle) + y * sinf(angle) >= settings.fovCos * sqrtf(x * x + y * y);
    }
};
// Reference-sized host scenarios remain available; production explicitly supplies the engine capacity.
typedef ControllerFor<18> Controller;
} // namespace iw3_bot_native
