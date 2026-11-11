#include "../../../src/game/iw3/mp/components/experimental/bot_native_controller.h"
#include <cstdlib>
#include <iostream>

#define CHECK(c) do { if (!(c)) { std::cerr << "FAIL line " << __LINE__ << ": " << #c << '\n'; std::exit(1); } } while (0)
using namespace iw3_bot_native;
struct World
{
    unsigned int calls;
    bool blocked[18][18];
    bool checked[18];
    World() : calls(0) { for (int i = 0; i < 18; ++i) { checked[i] = false; for (int j = 0; j < 18; ++j) blocked[i][j] = false; } }
    bool Visible(int self, int other, const Vec3 &, const Vec3 &)
    { ++calls; checked[self] = true; return !blocked[self][other]; }
};
Controller::Actor Player(float x, float y, int team)
{
    Controller::Actor a;
    a.alive = true; a.team = team;
    a.origin = Vec3{{x, y, 0}}; a.eye = Vec3{{x, y, 60}};
    a.angles[0] = a.angles[1] = a.angles[2] = 0;
    return a;
}
int main()
{
    Graph graph;
    CHECK(graph.Begin(8));
    for (int i = 0; i < 8; ++i) CHECK(graph.SetNode(i, Vec3{{static_cast<float>(i * 100), 0, 0}}, 0));
    for (int i = 0; i < 7; ++i) { CHECK(graph.Link(i, i + 1)); CHECK(graph.Link(i + 1, i)); }
    CHECK(graph.Finish());
    Controller c;
    Controller::Actor actors[18];
    World world;
    for (int i = 0; i < 18; ++i)
    {
        actors[i] = Player(static_cast<float>(i * 10), static_cast<float>(i % 3 * 10), i % 2 + 1);
        if (i != 17) c.Start(i, 0); // One human and seventeen native bots.
    }
    c.settings.fovCos = -1;
    for (int now = 0; now < 10000; now += 50)
    {
        const unsigned int sight = world.calls, paths = c.counters.paths;
        c.Frame(now, actors, graph, world);
        CHECK(world.calls - sight <= 6);
        CHECK(c.counters.paths - paths <= 1);
        for (int i = 0; i < 17; ++i)
        {
            const Controller::Output out = c.Input(i, now, actors[i], graph);
            CHECK(out.forward >= -127 && out.forward <= 127 && out.right >= -127 && out.right <= 127);
            CHECK(Finite(out.angles[0]) && Finite(out.angles[1]));
            CHECK(c.bots[i].target < 0 || actors[c.bots[i].target].team != actors[i].team);
            for (int k = 0; k < 3; ++k) actors[i].angles[k] = out.angles[k];
        }
    }
    for (int i = 0; i < 17; ++i) CHECK(world.checked[i]);
    CHECK(!world.checked[17]);
    CHECK(c.counters.acquisitions >= 17 && c.counters.sight <= 200 * 6);

    // A blocked early candidate must not starve another visible enemy or bypass the cone/range gates.
    c.Reset(); world = World();
    for (int i = 0; i < 18; ++i) actors[i] = Controller::Actor();
    actors[0] = Player(0, 0, 1); actors[1] = Player(500, 0, 2); actors[2] = Player(600, 0, 2);
    actors[3] = Player(100, 0, 1); // Teammate.
    actors[4] = Player(-500, 0, 2); // Behind.
    actors[5] = Player(10000, 0, 2); // Beyond range.
    c.settings.fovCos = 0; c.Start(0, 0); world.blocked[0][1] = true;
    c.Frame(0, actors, graph, world);
    CHECK(c.bots[0].target == -1 && world.calls == 1);
    c.Frame(100, actors, graph, world);
    CHECK(c.bots[0].target == 2 && world.calls == 2);
    Controller::Output out = c.Input(0, 100, actors[0], graph);
    CHECK(!out.fire && out.ads);
    out = c.Input(0, 399, actors[0], graph);
    CHECK(!out.fire); // A stale sight result cannot fire indefinitely.
    c.Frame(400, actors, graph, world);
    out = c.Input(0, 450, actors[0], graph);
    CHECK(out.fire);
    world.blocked[0][2] = true;
    c.Frame(500, actors, graph, world);
    CHECK(!c.bots[0].visible);
    CHECK(!c.Input(0, 500, actors[0], graph).fire);
    const Vec3 known = c.bots[0].lastKnown;
    actors[2].eye.v[1] = 900;
    c.Frame(600, actors, graph, world);
    CHECK(c.bots[0].lastKnown.v[1] == known.v[1]); // No tracking unseen live coordinates.
    actors[2].alive = false;
    c.Frame(650, actors, graph, world);
    CHECK(c.bots[0].target == -1);

    // FFA permits other free-team clients, and teammates never qualify in team modes.
    CHECK(Controller::Enemy(Player(0, 0, 0), Player(1, 0, 0)));
    CHECK(!Controller::Enemy(Player(0, 0, 1), Player(1, 0, 1)));
    CHECK(!Controller::Enemy(Player(0, 0, 0), Controller::Actor()));

    // Navigation consumes the reversed A* path, then replans after reaching it.
    c.Reset(); world = World();
    for (int i = 0; i < 18; ++i) actors[i] = Controller::Actor();
    actors[0] = Player(0, 0, 1); c.Start(0, 1000);
    c.bots[0].route.push_back(2); c.bots[0].route.push_back(1); c.bots[0].route.push_back(0);
    c.bots[0].routeIndex = 2; c.bots[0].nextPlan = 10000;
    out = c.Input(0, 1050, actors[0], graph);
    CHECK(out.forward == 127 && c.bots[0].routeIndex == 1);
    actors[0].origin.v[0] = 100; actors[0].eye.v[0] = 100;
    out = c.Input(0, 1100, actors[0], graph);
    CHECK(out.forward == 127 && c.bots[0].routeIndex == 0);
    actors[0].origin.v[0] = 200; actors[0].eye.v[0] = 200;
    out = c.Input(0, 1150, actors[0], graph);
    CHECK(out.forward == 0 && c.bots[0].routeIndex == -1);

    // Stuck recovery is bounded and does not require new collision queries.
    c.bots[0].routeIndex = 0; c.bots[0].route[0] = 7;
    for (int now = 2000; now <= 6000; now += 50) c.Input(0, now, actors[0], graph);
    CHECK(c.counters.stuck > 0 && c.bots[0].routeIndex == -1);
    c.Stop(0); CHECK(!c.bots[0].enabled && c.bots[0].route.empty() && c.bots[0].target == -1);
    c.Start(0, 7000);
    Graph missing;
    c.Frame(7000, actors, missing, world);
    out = c.Input(0, 7050, actors[0], missing);
    CHECK(!out.fire && out.forward == 0 && out.right == 0);
    actors[0].alive = false;
    CHECK(!c.Input(0, 7100, actors[0], graph).fire);
    std::cout << "PASS: 17-bot shared budgets/fairness, team/FOV/range filters, blocked-candidate rotation, reaction, "
                 "sight expiry, unseen-position memory, death, reversed routes, stuck recovery, reset and missing graph\n";
}
