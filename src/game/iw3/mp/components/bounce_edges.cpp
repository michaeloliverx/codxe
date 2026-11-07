#include "pch.h"
#include "bounce_edges.h"
#include "collision_geometry.h"
#include "debug_draw.h"

namespace iw3
{
namespace mp
{
namespace
{
namespace bounce_edge_geometry
{
using collision_geometry::Dot;
using collision_geometry::Normalize;
using collision_geometry::Point;

const int SAMPLE_COUNT = 5;
const int PROBES_PER_SAMPLE = 2 * 3 * 3;
const int PROBE_COUNT = SAMPLE_COUNT * PROBES_PER_SAMPLE;
const float TRACE_SKIN = 0.125f;

struct Edge
{
    Point points[2];
    float normals[2][3];
    int faceCount;
};

struct Probe
{
    Point start;
    Point end;
    Point target;
    float velocity[3];
    float sphereOffset[3];
    float radius;
};

inline float SampleFraction(int sample)
{
    return static_cast<float>(sample) / (SAMPLE_COUNT - 1);
}

inline bool IsCrease(const Edge &edge)
{
    if (edge.faceCount < 1 || edge.faceCount > 2)
        return false;
    return edge.faceCount == 1 || Dot(edge.normals[0], edge.normals[1]) < 0.9995f;
}

inline bool BuildProbe(const Edge &edge, int index, const float *mins, const float *maxs, Probe &probe)
{
    if (index < 0 || index >= PROBE_COUNT || !IsCrease(edge))
        return false;
    const int sample = index / PROBES_PER_SAMPLE;
    const int side = (index / 9) % 2;
    const int slope = (index / 3) % 3;
    const int direction = index % 3;
    if (side >= edge.faceCount || (sample != 0 && sample != SAMPLE_COUNT - 1 && direction != 1))
        return false;

    float tangent[3];
    float horizontal[3] = {edge.normals[side][0], edge.normals[side][1], 0.0f};
    for (int i = 0; i < 3; ++i)
        tangent[i] = edge.points[1].xyz[i] - edge.points[0].xyz[i];
    if (!Normalize(tangent) || !Normalize(horizontal))
        return false;
    if (direction != 1)
    {
        const float sign = direction == 0 ? -1.0f : 1.0f;
        const float x = horizontal[0];
        const float y = horizontal[1];
        horizontal[0] = 0.70710678f * (x - sign * y);
        horizontal[1] = 0.70710678f * (sign * x + y);
    }

    const float normalZ[3] = {0.2f, 0.5f, 0.65f};
    const float normalXY = sqrtf(1.0f - normalZ[slope] * normalZ[slope]);
    float normal[3] = {horizontal[0] * normalXY, horizontal[1] * normalXY, normalZ[slope]};
    if (sample != 0 && sample != SAMPLE_COUNT - 1)
    {
        const float along = Dot(normal, tangent);
        for (int i = 0; i < 3; ++i)
            normal[i] -= along * tangent[i];
        if (!Normalize(normal) || normal[2] <= 0.001f)
            return false;
    }

    const float halfWidth = 0.5f * (maxs[0] - mins[0]);
    const float halfHeight = 0.5f * (maxs[2] - mins[2]);
    probe.radius = halfWidth < halfHeight ? halfWidth : halfHeight;
    if (!(probe.radius > 0.0f))
        return false;
    probe.sphereOffset[0] = 0.5f * (mins[0] + maxs[0]);
    probe.sphereOffset[1] = 0.5f * (mins[1] + maxs[1]);
    probe.sphereOffset[2] = mins[2] + probe.radius;

    // Representative falling approaches, not a prediction of the player's entire jump.
    probe.velocity[0] = -horizontal[0] * 600.0f;
    probe.velocity[1] = -horizontal[1] * 600.0f;
    probe.velocity[2] = -100.0f;
    float movement[3] = {probe.velocity[0], probe.velocity[1], probe.velocity[2]};
    Normalize(movement);
    const float fraction = SampleFraction(sample);
    for (int i = 0; i < 3; ++i)
    {
        probe.target.xyz[i] = edge.points[0].xyz[i] + fraction * (edge.points[1].xyz[i] - edge.points[0].xyz[i]);
        const float center = probe.target.xyz[i] + normal[i] * (probe.radius + TRACE_SKIN) - probe.sphereOffset[i];
        probe.start.xyz[i] = center - movement[i] * 6.0f;
        probe.end.xyz[i] = center + movement[i] * 6.0f;
    }
    return true;
}

inline bool IsEdgeResponse(const Edge &edge, const Probe &probe, float fraction, bool startsolid, bool allsolid,
                           bool walkable, const float *normal)
{
    if (startsolid || allsolid || walkable || fraction <= 0.0f || fraction >= 1.0f || normal[2] <= 0.001f)
        return false;
    for (int i = 0; i < edge.faceCount; ++i)
        if (Dot(normal, edge.normals[i]) > 0.995f)
            return false;

    const float into = Dot(probe.velocity, normal);
    // PM_SlideMove uses this 1.001 overclip for an incoming velocity.
    if (into >= 0.0f || probe.velocity[2] - 1.001f * into * normal[2] <= 1.0f)
        return false;
    float errorSquared = 0.0f;
    for (int i = 0; i < 3; ++i)
    {
        const float contact = probe.start.xyz[i] + fraction * (probe.end.xyz[i] - probe.start.xyz[i]) +
                              probe.sphereOffset[i] - normal[i] * (probe.radius + TRACE_SKIN);
        const float error = contact - probe.target.xyz[i];
        errorSquared += error * error;
    }
    // Reject a nearer wall/face/edge which intercepted the probe before the requested contact.
    return errorSquared <= 0.75f * 0.75f;
}
} // namespace bounce_edge_geometry

using collision_geometry::BoundsDistanceSquared;
using collision_geometry::Cross;
using collision_geometry::Normalize;
using collision_geometry::Point;

const int PLAYER_SOLID = 0x2810011;
const int TRACE_BUDGET = 32;
const int PROBE_WORK_BUDGET = 512;
const int LINE_BUDGET = 128;
const float EDGE_COLOR[4] = {1.0f, 0.0f, 1.0f, 1.0f};

dvar_s *showEdges = nullptr;
dvar_s *edgeDistance = nullptr;

struct EdgeReference
{
    unsigned int key;
    unsigned int triangle;
    bool operator<(const EdgeReference &other) const
    {
        return key != other.key ? key < other.key : triangle < other.triangle;
    }
};

struct EdgeRecord
{
    unsigned int key;
    int triangles[2];
};

struct EdgeState
{
    unsigned short nextProbe;
    unsigned char samples;
};

struct Profile
{
    std::vector<EdgeState> states;
    float mins[3];
    float maxs[3];
    int mask;
    bool valid;
};

struct Candidate
{
    size_t index;
    float distanceSquared;
    bool operator<(const Candidate &other) const
    {
        return distanceSquared < other.distanceSquared;
    }
};

std::vector<EdgeRecord> edges;
Profile profiles[4];
const float (*mapVerts)[3] = nullptr;
const unsigned short *mapIndices = nullptr;
bool indexed = false;
std::string brushCollisionSetting;

void ClearEdges()
{
    std::vector<EdgeRecord>().swap(edges);
    for (int i = 0; i < 4; ++i)
    {
        std::vector<EdgeState>().swap(profiles[i].states);
        profiles[i].valid = false;
    }
    mapVerts = nullptr;
    mapIndices = nullptr;
    indexed = false;
}

void IndexEdges(const std::vector<unsigned int> &trees)
{
    ClearEdges();
    std::vector<EdgeReference> references;
    std::vector<unsigned char> seen(cm->triCount, 0);
    for (size_t i = 0; i < trees.size(); ++i)
    {
        const CollisionAabbTree &tree = cm->aabbTrees[trees[i]];
        if (!(cm->materials[tree.materialIndex].contentFlags & PLAYER_SOLID))
            continue;
        const CollisionPartition &partition = cm->partitions[tree.u.partitionIndex];
        if (partition.firstTri < 0 || partition.firstTri > cm->triCount - partition.triCount)
            continue;
        for (int tri = partition.firstTri; tri < partition.firstTri + partition.triCount; ++tri)
        {
            if (seen[tri])
                continue;
            seen[tri] = 1;
            const unsigned short *indices = &cm->triIndices[3 * tri];
            for (int side = 0; side < 3; ++side)
            {
                const unsigned int a = indices[side];
                const unsigned int b = indices[(side + 1) % 3];
                if (a == b || a >= cm->vertCount || b >= cm->vertCount)
                    continue;
                EdgeReference reference;
                reference.key = a < b ? (a << 16) | b : (b << 16) | a;
                reference.triangle = tri;
                references.push_back(reference);
            }
        }
    }
    std::sort(references.begin(), references.end());
    for (size_t i = 0; i < references.size();)
    {
        EdgeRecord edge;
        edge.key = references[i].key;
        edge.triangles[0] = references[i++].triangle;
        edge.triangles[1] = -1;
        while (i < references.size() && references[i].key == edge.key)
        {
            if (references[i].triangle != static_cast<unsigned int>(edge.triangles[0]))
                edge.triangles[1] = edge.triangles[1] == -1 ? static_cast<int>(references[i].triangle) : -2;
            ++i;
        }
        // More than two incident faces are ambiguous; do not report them as confirmed edge contacts.
        if (edge.triangles[1] != -2)
            edges.push_back(edge);
    }
    mapVerts = cm->verts;
    mapIndices = cm->triIndices;
    indexed = true;
}

bool GetEdge(size_t index, bounce_edge_geometry::Edge &edge)
{
    const EdgeRecord &record = edges[index];
    const unsigned int a = record.key >> 16;
    const unsigned int b = record.key & 0xFFFF;
    memcpy(edge.points[0].xyz, cm->verts[a], sizeof(edge.points[0].xyz));
    memcpy(edge.points[1].xyz, cm->verts[b], sizeof(edge.points[1].xyz));
    edge.faceCount = record.triangles[1] < 0 ? 1 : 2;
    for (int i = 0; i < edge.faceCount; ++i)
    {
        const unsigned short *tri = &cm->triIndices[3 * record.triangles[i]];
        if (tri[0] >= cm->vertCount || tri[1] >= cm->vertCount || tri[2] >= cm->vertCount)
            return false;
        float first[3], second[3];
        for (int axis = 0; axis < 3; ++axis)
        {
            first[axis] = cm->verts[tri[2]][axis] - cm->verts[tri[0]][axis];
            second[axis] = cm->verts[tri[1]][axis] - cm->verts[tri[0]][axis];
        }
        Cross(first, second, edge.normals[i]);
        if (!Normalize(edge.normals[i]))
            return false;
    }
    return bounce_edge_geometry::IsCrease(edge);
}

void UpdateBounds(int localClientNum, Profile &profile)
{
    const float standingMins[3] = {-15.0f, -15.0f, 0.0f};
    const float standingMaxs[3] = {15.0f, 15.0f, 70.0f};
    const float *mins = standingMins;
    const float *maxs = standingMaxs;
    int mask = PLAYER_SOLID;
    const cg_s &view = (*cgArray)[localClientNum];
    if (cg_pmove->ps == &view.predictedPlayerState && cg_pmove->maxs[0] > cg_pmove->mins[0] &&
        cg_pmove->maxs[2] > cg_pmove->mins[2])
    {
        mins = cg_pmove->mins;
        maxs = cg_pmove->maxs;
        mask = cg_pmove->tracemask & PLAYER_SOLID;
        if (!mask)
            mask = PLAYER_SOLID;
    }
    if (!profile.valid || profile.states.size() != edges.size() || profile.mask != mask ||
        memcmp(profile.mins, mins, sizeof(profile.mins)) || memcmp(profile.maxs, maxs, sizeof(profile.maxs)))
    {
        profile.states.assign(edges.size(), EdgeState());
        memcpy(profile.mins, mins, sizeof(profile.mins));
        memcpy(profile.maxs, maxs, sizeof(profile.maxs));
        profile.mask = mask;
        profile.valid = true;
    }
}

void ProbeEdge(size_t index, Profile &profile, int &traces, int &work)
{
    EdgeState &state = profile.states[index];
    if (state.nextProbe >= bounce_edge_geometry::PROBE_COUNT)
        return;
    bounce_edge_geometry::Edge edge;
    if (!GetEdge(index, edge))
    {
        state.nextProbe = bounce_edge_geometry::PROBE_COUNT;
        ++work;
        return;
    }
    while (state.nextProbe < bounce_edge_geometry::PROBE_COUNT && traces < TRACE_BUDGET && work < PROBE_WORK_BUDGET)
    {
        const int probeIndex = state.nextProbe++;
        ++work;
        const int sample = probeIndex / bounce_edge_geometry::PROBES_PER_SAMPLE;
        if (state.samples & (1 << sample))
            continue;
        bounce_edge_geometry::Probe probe;
        if (!bounce_edge_geometry::BuildProbe(edge, probeIndex, profile.mins, profile.maxs, probe))
            continue;
        trace_t trace;
        CM_BoxTrace(&trace, probe.start.xyz, probe.end.xyz, profile.mins, profile.maxs, 0, profile.mask);
        ++traces;
        if (bounce_edge_geometry::IsEdgeResponse(edge, probe, trace.fraction, trace.startsolid, trace.allsolid,
                                                 trace.walkable, trace.normal))
            state.samples |= static_cast<unsigned char>(1 << sample);
    }
}

void DrawMarkers(const std::vector<Candidate> &candidates, const Profile &profile)
{
    GfxPointVertex vertices[128];
    unsigned char color[4];
    DebugDraw::PackColor(EDGE_COLOR, color);
    int batch = 0;
    int lines = 0;
    for (size_t i = 0; i < candidates.size() && lines < LINE_BUDGET; ++i)
    {
        const size_t index = candidates[i].index;
        if (!profile.states[index].samples)
            continue;
        bounce_edge_geometry::Edge edge;
        if (!GetEdge(index, edge))
            continue;
        float lengthSquared = 0.0f;
        for (int axis = 0; axis < 3; ++axis)
        {
            const float delta = edge.points[1].xyz[axis] - edge.points[0].xyz[axis];
            lengthSquared += delta * delta;
        }
        const float length = sqrtf(lengthSquared);
        if (length <= 0.001f)
            continue;
        const float half = (std::min)(0.125f, 2.0f / length);
        for (int sample = 0; sample < bounce_edge_geometry::SAMPLE_COUNT && lines < LINE_BUDGET; ++sample)
        {
            if (!(profile.states[index].samples & (1 << sample)))
                continue;
            const float center = bounce_edge_geometry::SampleFraction(sample);
            const float fractions[2] = {(std::max)(0.0f, center - half), (std::min)(1.0f, center + half)};
            for (int point = 0; point < 2; ++point)
            {
                GfxPointVertex &vertex = vertices[2 * batch + point];
                for (int axis = 0; axis < 3; ++axis)
                    vertex.xyz[axis] = edge.points[0].xyz[axis] +
                                       fractions[point] * (edge.points[1].xyz[axis] - edge.points[0].xyz[axis]);
                memcpy(vertex.color, color, sizeof(color));
            }
            ++batch;
            ++lines;
            if (batch == 64)
            {
                if (!DebugDraw::DrawLines(vertices, batch))
                    return;
                batch = 0;
            }
        }
    }
    if (batch)
        DebugDraw::DrawLines(vertices, batch);
}
} // namespace

void bounce_edges::RegisterDvars()
{
    showEdges = Dvar_RegisterBool("cm_showBounceEdges", true, DVAR_FLAG_NONE,
                                  "With cm_showBounces, mark probed capsule edge contacts in magenta");
    edgeDistance = Dvar_RegisterInt("cm_showBounceEdgeDist", 256, 64, 1000, DVAR_FLAG_NONE,
                                    "Maximum distance for capsule edge bounce probes");
}

void bounce_edges::Reset()
{
    if (indexed)
        ClearEdges();
}

void bounce_edges::Draw(int localClientNum, const std::vector<unsigned int> &worldTerrain, float drawDistance)
{
    if (!showEdges || !showEdges->current.enabled)
    {
        if (indexed)
            ClearEdges();
        return;
    }
    if (localClientNum < 0 || localClientNum >= 4 || !*cgArray || !cm->isInUse || !cm->leafs || !cm->materials ||
        !cm->partitions || !cm->aabbTrees || !cm->verts || !cm->triIndices || cm->triCount <= 0)
    {
        if (indexed)
            ClearEdges();
        return;
    }
    if (!indexed || mapVerts != cm->verts || mapIndices != cm->triIndices)
        IndexEdges(worldTerrain);
    const char *setting = Dvar_GetString("noclip_brushes");
    if (brushCollisionSetting != (setting ? setting : ""))
    {
        brushCollisionSetting = setting ? setting : "";
        for (int i = 0; i < 4; ++i)
            profiles[i].valid = false;
    }
    Profile &profile = profiles[localClientNum];
    UpdateBounds(localClientNum, profile);
    const float *origin = (*cgArray)[localClientNum].refdef.vieworg;
    const float distance = (std::min)(static_cast<float>(edgeDistance->current.integer), drawDistance);
    std::vector<Candidate> candidates;
    for (size_t i = 0; i < edges.size(); ++i)
    {
        const EdgeRecord &edge = edges[i];
        const float *a = cm->verts[edge.key >> 16];
        const float *b = cm->verts[edge.key & 0xFFFF];
        float mins[3], maxs[3];
        for (int axis = 0; axis < 3; ++axis)
        {
            mins[axis] = (std::min)(a[axis], b[axis]);
            maxs[axis] = (std::max)(a[axis], b[axis]);
        }
        const float squared = BoundsDistanceSquared(origin, mins, maxs);
        if (squared <= distance * distance)
        {
            Candidate candidate = {i, squared};
            candidates.push_back(candidate);
        }
    }
    std::sort(candidates.begin(), candidates.end());
    int traces = 0;
    int work = 0;
    for (size_t i = 0; i < candidates.size() && traces < TRACE_BUDGET && work < PROBE_WORK_BUDGET; ++i)
        ProbeEdge(candidates[i].index, profile, traces, work);
    DrawMarkers(candidates, profile);
}

} // namespace mp
} // namespace iw3
