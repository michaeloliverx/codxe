#include "../../../src/game/iw3/mp/components/experimental/bot_native_controller.h"
#include <cstdlib>
#include <iostream>
#include <climits>

#define CHECK(c) do { if (!(c)) { std::cerr << "FAIL line " << __LINE__ << ": " << #c << '\n'; std::exit(1); } } while (0)
using namespace iw3_bot_native;

template <int Capacity> struct World
{
    unsigned int calls;
    bool checked[Capacity];
    World() : calls(0) { for (int i = 0; i < Capacity; ++i) checked[i] = false; }
    bool Visible(int self, int other, const Vec3 &, const Vec3 &)
    {
        CHECK(self > 0 && self < Capacity && other >= 0 && other < Capacity && self != other);
        checked[self] = true;
        ++calls;
        return true;
    }
};

template <int Capacity> void CheckCapacity()
{
    typedef ControllerFor<Capacity> Bots;
    Bots bots;
    typename Bots::Actor actors[Capacity];
    World<Capacity> world;
    Graph graph;
    CHECK(graph.Begin(2));
    CHECK(graph.SetNode(0, Vec3{{0, 0, 0}}, 0));
    CHECK(graph.SetNode(1, Vec3{{1000, 0, 0}}, 0));
    CHECK(graph.Link(0, 1) && graph.Link(1, 0) && graph.Finish());
    for (int i = 0; i < Capacity; ++i)
    {
        actors[i].alive = true;
        actors[i].origin.v[0] = actors[i].eye.v[0] = static_cast<float>(i * 10);
        actors[i].eye.v[2] = 60;
    }
    int now = 0;
    // Add clients progressively into every slot, including the old out-of-bounds slots 18..23.
    for (int i = 1; i < Capacity; ++i, now += 50)
    {
        CHECK(bots.Start(i, now));
        bots.Frame(now, actors, graph, world);
        bots.Input(i, now, actors[i], graph);
        CHECK(bots.settings.sightBudget == 6 && bots.settings.pathBudget == 1 && bots.settings.reactionMs == 250);
    }
    for (int frame = 0; frame < 100; ++frame, now += 50)
    {
        const unsigned int sight = world.calls, paths = bots.counters.paths;
        bots.Frame(now, actors, graph, world);
        CHECK(world.calls - sight <= 6 && bots.counters.paths - paths <= 1);
        for (int i = 1; i < Capacity; ++i)
        {
            const typename Bots::Output out = bots.Input(i, now, actors[i], graph);
            CHECK(out.forward >= -127 && out.forward <= 127 && out.right >= -127 && out.right <= 127);
            CHECK(Finite(out.angles[0]) && Finite(out.angles[1]));
        }
    }
    CHECK(!world.checked[0]);
    for (int i = 1; i < Capacity; ++i) CHECK(world.checked[i]);
    const int invalid[] = {-1, Capacity, INT_MAX};
    for (int i = 0; i < 3; ++i)
    {
        CHECK(!bots.Start(invalid[i], now) && !bots.Stop(invalid[i]));
        const typename Bots::Output out = bots.Input(invalid[i], now, actors[0], graph);
        CHECK(!out.fire && out.forward == 0 && out.right == 0);
    }
    // Reuse the highest slot repeatedly after its route has been allocated.
    for (int reuse = 0; reuse < 100; ++reuse, now += 50)
    {
        CHECK(bots.Stop(Capacity - 1) && bots.Start(Capacity - 1, now));
        bots.Frame(now, actors, graph, world);
        bots.Input(Capacity - 1, now, actors[Capacity - 1], graph);
    }
    CHECK(bots.settings.sightBudget == 6 && bots.settings.reactionMs == 250);
    std::cout << "PASS: " << Capacity << " slots, progressive addition, highest-slot reuse, invalid indices and shared budgets\n";
}

int main()
{
    CheckCapacity<18>();
    CheckCapacity<24>();
    CheckCapacity<32>();
}
