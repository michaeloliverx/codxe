#include "../../../src/game/iw3/mp/components/experimental/bot_native_core.h"
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

using namespace iw3_bot_native;

// A direct translation of the GSC heap's one-based indexing and object references.
// Deliberately independent from Graph's zero-based heap implementation.
std::vector<int> ScriptSearch(const Graph &graph, int start, int goal, int team, bool greedy)
{
    struct Object
    {
        float g = FLT_MAX, f = FLT_MAX;
        int parent = -1;
        bool open = false, closed = false;
    };
    std::vector<Object> objects(graph.nodes.size());
    std::vector<int> heap;
    auto compare = [&](int a, int b) { return objects[a].f < objects[b].f; };
    auto insert = [&](int item)
    {
        int current = static_cast<int>(heap.size()) + 1;
        heap.push_back(item);
        while (current > 1)
        {
            const int last = current;
            current /= 2;
            if (!compare(item, heap[current - 1]))
                break;
            heap[last - 1] = heap[current - 1];
            heap[current - 1] = item;
        }
    };
    auto remove = [&]()
    {
        const int item = heap[0], move = heap.back();
        heap[0] = move;
        heap.pop_back();
        const int count = static_cast<int>(heap.size());
        int last = 1;
        while (last * 2 <= count)
        {
            int next = last * 2;
            if (next + 1 <= count && !compare(heap[next - 1], heap[next]))
                ++next;
            if (compare(move, heap[next - 1]))
                break;
            heap[last - 1] = heap[next - 1];
            heap[next - 1] = move;
            last = next;
        }
        return item;
    };
    objects[start].g = 0;
    objects[start].f = DistanceSquared(graph.nodes[start].origin, graph.nodes[goal].origin);
    objects[start].open = true;
    insert(start);
    unsigned int expansions = 0;
    while (!heap.empty())
    {
        assert(++expansions <= Graph::MaxExpansions);
        const int best = remove();
        objects[best].open = false;
        if (best == goal)
        {
            std::vector<int> path;
            for (int index = goal; index != -1; index = objects[index].parent)
            {
                assert(path.size() < graph.nodes.size());
                path.push_back(index);
            }
            return path;
        }
        const auto &wp = graph.nodes[best];
        for (int i = static_cast<int>(wp.children.size()) - 1; i >= 0; --i)
        {
            const int child = wp.children[i];
            const auto &next = graph.nodes[child];
            int penalty = 1;
            if (!greedy && team >= 0 && next.usage[team] > 1)
                penalty = next.usage[team];
            penalty += next.extraCost;
            const float newg = objects[best].g + DistanceSquared(wp.origin, next.origin) * penalty;
            auto &object = objects[child];
            if ((object.open || object.closed) && object.g <= newg)
                continue;
            object.parent = best;
            object.g = newg;
            object.f = newg + DistanceSquared(next.origin, graph.nodes[goal].origin);
            object.closed = false;
            if (!object.open)
            {
                object.open = true;
                insert(child);
            }
        }
        objects[best].closed = true;
    }
    return {};
}

void AimTests()
{
    const Vec3 eye = {{0, 0, 60}}, target = {{500, -200, 100}};
    const Vec3 zero = {{0, 0, 0}}, own = {{20, 50, 0}}, velocity = {{0, -30, 0}};
    const float current[3] = {5, 179, 0};
    Aim aim;
    aim.Start(eye, target, current, own, velocity, 0.3f, true, 100);
    assert(aim.Tick(100));
    assert(aim.remaining == 5);
    const float first = aim.angle[1];
    assert(!aim.Tick(149));
    assert(aim.angle[1] == first);
    assert(aim.Tick(150));
    assert(aim.Tick(350));
    assert(!aim.active && aim.remaining == 0 && !aim.Tick(500));
    const float x = target.v[0] - eye.v[0] + (velocity.v[0] - own.v[0]) * 0.25f -
                    cosf(current[0] * 0.017453292519943295f) * cosf(current[1] * 0.017453292519943295f);
    const float y = target.v[1] - eye.v[1] + (velocity.v[1] - own.v[1]) * 0.25f -
                    cosf(current[0] * 0.017453292519943295f) * sinf(current[1] * 0.017453292519943295f);
    assert(fabsf(Angle180(aim.angle[1] - atan2f(y, x) / 0.017453292519943295f)) < 0.001f);
    aim.Start(eye, target, current, zero, zero, 0, false, 0xFFFFFFF0u);
    assert(aim.Tick(0xFFFFFFF0u) && !aim.active);
    aim.Start(eye, target, current, zero, zero, 0.2f, false, 0xFFFFFFF0u);
    assert(aim.Tick(0xFFFFFFF0u) && !aim.Tick(0) && aim.Tick(34));
    // Replacing or cancelling a request must never retain interpolation from its predecessor.
    aim.Start(eye, Vec3{{-500, 20, 60}}, current, zero, zero, 0.05f, false, 1000);
    assert(aim.Tick(1000) && !aim.active);
    aim.active = false;
    assert(!aim.Tick(2000));
    assert(Angle180(180) == 180 && Angle180(-180) == 180);
    assert(fabsf(Angle180(-181) - 179) < 0.0001f && fabsf(Angle180(721) - 1) < 0.0001f);
    const float straight[3] = {0, 0, 0};
    aim.Start(Vec3{{0, 0, 0}}, Vec3{{1, 0, 0}}, straight, zero, zero, 0, false, 0);
    assert(aim.Tick(0) && aim.angle[0] == 90); // Stock vectoangles' zero-vector case.
}

int main(int argc, char **argv)
{
    AimTests();
    Graph graph;
    assert(!graph.Begin(0) && !graph.Begin(Graph::MaxNodes + 1));
    assert(graph.Begin(3));
    assert(!graph.Finish());
    assert(graph.SetNode(0, Vec3{{0, 0, 0}}, 0));
    assert(!graph.SetNode(0, Vec3{{1, 1, 1}}, 0));
    assert(!graph.SetNode(1, Vec3{{NAN, 0, 0}}, 0));
    assert(graph.SetNode(1, Vec3{{10, 0, 0}}, 4));
    assert(graph.SetNode(2, Vec3{{10, 0, 0}}, 0));
    assert(!graph.Link(0, 3) && graph.Link(0, 1) && graph.Finish());
    assert(!graph.Link(0, 2));
    assert(graph.Nearest(Vec3{{10, 0, 0}}) == 1);
    std::vector<int> path;
    assert(graph.Search(0, 1, 0, false, path) && path == std::vector<int>({1, 0}));
    assert(graph.Search(1, 0, -1, false, path) && path.empty());
    assert(graph.Search(2, 2, -1, false, path) && path == std::vector<int>({2}));
    assert(!graph.Search(-1, 2, 0, false, path) && !graph.Search(0, 2, 2, false, path));
    assert(graph.Usage(1, 0, -1) && graph.nodes[1].usage[0] == 0);
    assert(!graph.Usage(1, 2, 1) && !graph.Usage(1, 0, 2));
    graph.Clear();
    assert(!graph.ready && graph.nodes.empty() && !graph.Search(0, 0, 0, false, path));

    std::mt19937 rng(0x1A3360);
    size_t maps = 0, nodes = 0, links = 0, comparisons = 0, rejected = 0;
    double nativeMicros = 0;
    unsigned int maxExpansions = 0;
    for (int arg = 1; arg < argc; ++arg)
    {
        std::ifstream input(argv[arg]);
        assert(input);
        std::string line;
        std::getline(input, line);
        const int count = std::stoi(line);
        if (!graph.Begin(count))
        {
            std::cerr << "Unsupported graph " << argv[arg] << '\n';
            return 2;
        }
        bool valid = true;
        for (int i = 0; i < count; ++i)
        {
            assert(static_cast<bool>(std::getline(input, line)));
            std::stringstream row(line);
            std::string origin, children, type;
            std::getline(row, origin, ',');
            std::getline(row, children, ',');
            std::getline(row, type, ',');
            Vec3 position;
            std::stringstream coordinates(origin);
            assert(static_cast<bool>(coordinates >> position.v[0] >> position.v[1] >> position.v[2]));
            assert(graph.SetNode(i, position, type == "climb" || type == "prone" ? 4 : 0));
            std::stringstream connections(children);
            std::string token;
            while (connections >> token)
            {
                const int child = std::stoi(token); // GSC int(string) truncates fractional CSV tokens.
                if (!graph.Link(i, child))
                    valid = false;
                ++links;
            }
        }
        if (!valid)
        {
            std::cout << "Rejected malformed graph (script fallback): " << argv[arg] << '\n';
            ++rejected;
            continue;
        }
        assert(graph.Finish());
        for (int query = 0; query < 300; ++query)
        {
            const int start = rng() % count, goal = rng() % count, team = query % 3 - 1;
            const bool greedy = query % 4 == 0;
            if (query % 5 == 0)
                for (int i = 0; i < count; ++i)
                    for (int t = 0; t < 2; ++t)
                        graph.nodes[i].usage[t] = rng() % 7;
            const auto reference = ScriptSearch(graph, start, goal, team, greedy);
            const auto begin = std::chrono::steady_clock::now();
            assert(graph.Search(start, goal, team, greedy, path));
            nativeMicros += std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - begin).count();
            if (path != reference)
            {
                std::cerr << "Script path mismatch: " << argv[arg] << " query " << start << " -> " << goal << '\n';
                return 3;
            }
            maxExpansions = (std::max)(maxExpansions, graph.expansions);
            ++comparisons;
        }
        ++maps;
        nodes += count;
    }
    std::cout << "PASS: lifecycle, validation, directed/unreachable/tie routes, aim "
                 "prediction/wrap/replacement/cancellation\n";
    std::cout << maps << " maps, " << nodes << " nodes, " << links << " directed links, " << comparisons
              << " exact path comparisons against script translation\n";
    std::cout << rejected << " malformed graphs rejected for script fallback\n";
    if (comparisons)
        std::cout << "Host native search average " << nativeMicros / comparisons << " us, max expansions "
                  << maxExpansions << " (host only; not a Xenon speedup measurement)\n";
}
