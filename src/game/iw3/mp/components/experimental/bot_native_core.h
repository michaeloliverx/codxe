#pragma once

// Engine-independent parts of the IW3 Bot Warfare experiment. Keep compatible with the Xenon compiler.
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <vector>

namespace iw3_bot_native
{
struct Vec3
{
    float v[3];
};

inline bool Finite(float value)
{
    return value >= -FLT_MAX && value <= FLT_MAX;
}

inline float DistanceSquared(const Vec3 &a, const Vec3 &b)
{
    const float x = a.v[0] - b.v[0], y = a.v[1] - b.v[1], z = a.v[2] - b.v[2];
    return x * x + y * y + z * z;
}

inline float Angle180(float angle)
{
    const float fraction = angle / 360.0f;
    const float wrapped = (fraction - floorf(fraction)) * 360.0f;
    return wrapped > 180.0f ? wrapped - 360.0f : wrapped;
}

struct Aim
{
    bool active;
    unsigned int nextTime;
    int remaining;
    float angle[3];
    float step[2];

    Aim() : active(false), nextTime(0), remaining(0)
    {
    }

    void Start(Vec3 eye, Vec3 position, const float *current, const Vec3 &ownVelocity, const Vec3 &targetVelocity,
               float seconds, bool predict, unsigned int now)
    {
        int steps = static_cast<int>(seconds * 20.0f);
        if (steps < 1)
            steps = 1;
        const float prediction = predict ? 0.05f * (steps - 1) : 0.0f;
        const float radians = 0.017453292519943295f;
        // Match bot_lookat's (pos - eye) - anglesToForward(current), including its one-unit offset.
        const float forward[3] = {cosf(current[0] * radians) * cosf(current[1] * radians),
                                  cosf(current[0] * radians) * sinf(current[1] * radians), -sinf(current[0] * radians)};
        float direction[3];
        for (int i = 0; i < 3; ++i)
            direction[i] =
                position.v[i] - eye.v[i] + (targetVelocity.v[i] - ownVelocity.v[i]) * prediction - forward[i];
        const float yaw = atan2f(direction[1], direction[0]) / radians;
        const float pitch =
            direction[0] == 0 && direction[1] == 0
                ? (direction[2] > 0 ? -90.0f : 90.0f)
                : -atan2f(direction[2], sqrtf(direction[0] * direction[0] + direction[1] * direction[1])) / radians;
        step[0] = Angle180(pitch - current[0]) / steps;
        step[1] = Angle180(yaw - current[1]) / steps;
        angle[0] = current[0];
        angle[1] = current[1];
        angle[2] = 0;
        remaining = steps;
        nextTime = now;
        active = true;
    }

    bool Tick(unsigned int now)
    {
        const int elapsed = static_cast<int>(now - nextTime);
        if (!active || elapsed < 0)
            return false;
        int count = elapsed / 50 + 1;
        if (count > remaining)
            count = remaining;
        angle[0] = Angle180(angle[0] + step[0] * count);
        angle[1] = Angle180(angle[1] + step[1] * count);
        remaining -= count;
        nextTime += count * 50;
        active = remaining > 0;
        return true;
    }
};

class Graph
{
  public:
    enum
    {
        MaxNodes = 8192,
        MaxEdges = 65536,
        MaxExpansions = 65536
    };
    struct Node
    {
        Vec3 origin;
        int extraCost;
        int traversal; // Native controller only: jump=1, crouch=2, prone=4, climb=8.
        int usage[2];
        bool defined;
        std::vector<int> children;
        Node() : extraCost(0), traversal(0), defined(false)
        {
            usage[0] = usage[1] = 0;
        }
    };
    std::vector<Node> nodes;
    bool ready;
    unsigned int expansions;

    Graph() : ready(false), expansions(0), edges(0)
    {
    }

    bool Begin(int count)
    {
        Clear();
        if (count < 1 || count > MaxNodes)
            return false;
        nodes.resize(count);
        scratch.resize(count);
        heap.reserve(count);
        return true;
    }

    void Clear()
    {
        ready = false;
        edges = 0;
        expansions = 0;
        std::vector<Node>().swap(nodes);
        std::vector<SearchNode>().swap(scratch);
        std::vector<int>().swap(heap);
    }

    bool SetNode(int index, const Vec3 &origin, int extraCost)
    {
        if (ready || !Valid(index) || nodes[index].defined || (extraCost != 0 && extraCost != 4))
            return false;
        for (int i = 0; i < 3; ++i)
            if (!Finite(origin.v[i]) || fabsf(origin.v[i]) > 1000000.0f)
                return false;
        nodes[index].origin = origin;
        nodes[index].extraCost = extraCost;
        nodes[index].defined = true;
        return true;
    }

    bool Link(int from, int to)
    {
        if (ready || !Valid(from) || !Valid(to) || edges >= MaxEdges)
            return false;
        nodes[from].children.push_back(to);
        ++edges;
        return true;
    }

    bool Finish()
    {
        if (nodes.empty())
            return false;
        for (size_t i = 0; i < nodes.size(); ++i)
            if (!nodes[i].defined)
                return false;
        ready = true;
        return true;
    }

    bool Valid(int index) const
    {
        return index >= 0 && index < static_cast<int>(nodes.size());
    }

    int Nearest(const Vec3 &origin) const
    {
        int candidate = -1;
        float distance = 2147483647.0f;
        // Reverse iteration and <= preserve the script's tie choice and distance cutoff.
        for (int i = static_cast<int>(nodes.size()) - 1; i >= 0; --i)
        {
            const float current = DistanceSquared(nodes[i].origin, origin);
            if (current <= distance)
            {
                distance = current;
                candidate = i;
            }
        }
        return candidate;
    }

    bool Usage(int index, int team, int delta)
    {
        if (!ready || !Valid(index) || team < 0 || team > 1 || (delta != -1 && delta != 1))
            return false;
        int &count = nodes[index].usage[team];
        if (delta > 0 && count >= 1000000)
            return false;
        count = (std::max)(0, count + delta);
        return true;
    }

    // false means an invalid/budget-limited request: the script must fall back. An empty successful path is
    // unreachable.
    bool Search(int start, int goal, int team, bool greedy, std::vector<int> &path)
    {
        path.clear();
        expansions = 0;
        if (!ready || !Valid(start) || !Valid(goal) || team < -1 || team > 1)
            return false;
        for (size_t i = 0; i < scratch.size(); ++i)
            scratch[i] = SearchNode();
        heap.clear();
        scratch[start].g = 0;
        scratch[start].f = DistanceSquared(nodes[start].origin, nodes[goal].origin);
        Push(start);
        while (!heap.empty())
        {
            if (++expansions > MaxExpansions)
                return false;
            const int best = Pop();
            if (best == goal)
            {
                for (int index = goal; index != -1; index = scratch[index].parent)
                {
                    if (path.size() >= nodes.size())
                    {
                        path.clear();
                        return false;
                    }
                    path.push_back(index);
                }
                return true; // goal -> start, as consumed by doWalk.
            }
            const Node &node = nodes[best];
            for (int i = static_cast<int>(node.children.size()) - 1; i >= 0; --i)
            {
                const int child = node.children[i];
                const Node &next = nodes[child];
                const int usage = !greedy && team >= 0 ? next.usage[team] : 0;
                const float penalty = static_cast<float>((std::max)(1, usage) + next.extraCost);
                const float cost = scratch[best].g + DistanceSquared(node.origin, next.origin) * penalty;
                SearchNode &entry = scratch[child];
                if (cost >= entry.g)
                    continue;
                entry.parent = best;
                entry.g = cost;
                entry.f = cost + DistanceSquared(next.origin, nodes[goal].origin);
                // Retain Bot Warfare's heap semantics: changing an open node does not reheapify it.
                // This is intentional compatibility, not a claim of shortest-path optimality.
                if (!entry.open)
                    Push(child);
            }
        }
        return true;
    }

  private:
    struct SearchNode
    {
        float g, f;
        int parent;
        bool open;
        SearchNode() : g(FLT_MAX), f(FLT_MAX), parent(-1), open(false)
        {
        }
    };
    std::vector<SearchNode> scratch;
    std::vector<int> heap;
    int edges;

    void Push(int node)
    {
        scratch[node].open = true;
        heap.push_back(node);
        size_t index = heap.size() - 1;
        while (index > 0)
        {
            const size_t parent = (index - 1) / 2;
            if (scratch[heap[parent]].f <= scratch[node].f)
                break;
            heap[index] = heap[parent];
            index = parent;
        }
        heap[index] = node;
    }

    int Pop()
    {
        const int result = heap[0];
        const int last = heap.back();
        heap.pop_back();
        scratch[result].open = false;
        if (!heap.empty())
        {
            size_t index = 0;
            while (index * 2 + 1 < heap.size())
            {
                size_t child = index * 2 + 1;
                if (child + 1 < heap.size() && scratch[heap[child + 1]].f <= scratch[heap[child]].f)
                    ++child;
                if (scratch[last].f < scratch[heap[child]].f)
                    break;
                heap[index] = heap[child];
                index = child;
            }
            heap[index] = last;
        }
        return result;
    }
};
} // namespace iw3_bot_native
