#include "pch.h"
#include "collision.h"
#include "bounce_edges.h"
#include "collision_geometry.h"
#include "debug_draw.h"

namespace iw3
{
namespace mp
{
namespace
{
using namespace collision_geometry;

const int PLAYER_SOLID = 0x2810011;
const size_t MAX_CACHED_BRUSHES = 256;
const size_t MAX_CACHED_POINTS = 65536;
const size_t MAX_CACHED_MODELS = 64;
const int SOLID_BMODEL = 0xFFFFFF;
const int ENTITY_COLLISION_DISABLED = 1;
const int DYNENT_DRAW_BRUSH = 1;

dvar_s *showCollision = nullptr;
dvar_s *showBounces = nullptr;
dvar_s *drawDistance = nullptr;
dvar_s *maxLines = nullptr;

struct Face
{
    std::vector<Point> points;
    float normal[3];
    float mins[3];
    float maxs[3];
};

struct CachedBrush
{
    std::vector<Face> faces;
    size_t pointCount;
    unsigned int used;
};

struct Candidate
{
    unsigned int index;
    enum Type
    {
        BRUSH,
        TERRAIN,
        MODEL
    } type;
    float distanceSquared;

    bool operator<(const Candidate &other) const
    {
        return distanceSquared < other.distanceSquared;
    }
};

struct CachedModel
{
    std::vector<unsigned short> brushes;
    std::vector<unsigned int> terrain;
    unsigned int used;
};

struct ModelInstance
{
    unsigned int model;
    Transform transform;
};

std::vector<unsigned short> worldBrushes;
std::vector<unsigned int> worldTerrain;
std::map<unsigned int, CachedBrush> brushCache;
std::map<unsigned int, CachedModel> modelCache;
size_t cachedPoints = 0;
unsigned int drawSerial = 0;
const cbrush_t *mapBrushes = nullptr;
const CollisionAabbTree *mapTrees = nullptr;
const cmodel_t *mapModels = nullptr;
bool indexed = false;

void ClearMap()
{
    bounce_edges::Reset();
    std::vector<unsigned short>().swap(worldBrushes);
    std::vector<unsigned int>().swap(worldTerrain);
    brushCache.clear();
    modelCache.clear();
    cachedPoints = 0;
    drawSerial = 0;
    mapBrushes = nullptr;
    mapTrees = nullptr;
    mapModels = nullptr;
    indexed = false;
}

void IndexWorld()
{
    ClearMap();
    mapBrushes = cm->brushes;
    mapTrees = cm->aabbTrees;
    mapModels = cm->cmodels;
    std::vector<unsigned char> brushes(cm->numBrushes, 0);
    std::vector<unsigned char> nodes(cm->leafbrushNodesCount, 0);
    std::vector<unsigned char> trees(cm->aabbTreeCount, 0);
    std::vector<unsigned int> pending;

    // Only BSP world leaves: inline brush models need entity transforms and must not appear at the map origin.
    for (unsigned int leaf = 0; leaf < cm->numLeafs; ++leaf)
    {
        const cLeaf_t &worldLeaf = cm->leafs[leaf];
        if (worldLeaf.leafBrushNode > 0 && cm->leafbrushNodes)
            pending.push_back(worldLeaf.leafBrushNode);
        while (!pending.empty())
        {
            const unsigned int index = pending.back();
            pending.pop_back();
            if (index >= nodes.size() || nodes[index])
                continue;
            nodes[index] = 1;
            const cLeafBrushNode_s &node = cm->leafbrushNodes[index];
            if (node.leafBrushCount > 0)
            {
                for (int i = 0; i < node.leafBrushCount; ++i)
                {
                    const unsigned int brush = node.data.leaf.brushes[i];
                    if (brush < brushes.size())
                        brushes[brush] = 1;
                }
            }
            else
            {
                // Negative counts also contain an overlapping subtree immediately after this node.
                if (node.leafBrushCount < 0)
                    pending.push_back(index + 1);
                for (int child = 0; child < 2; ++child)
                    if (node.data.children.childOffset[child])
                        pending.push_back(index + node.data.children.childOffset[child]);
            }
        }

        if (!cm->aabbTrees)
            continue;
        for (unsigned int i = 0; i < worldLeaf.collAabbCount; ++i)
            pending.push_back(worldLeaf.firstCollAabbIndex + i);
        while (!pending.empty())
        {
            const unsigned int index = pending.back();
            pending.pop_back();
            if (index >= trees.size() || trees[index])
                continue;
            trees[index] = 1;
            const CollisionAabbTree &tree = cm->aabbTrees[index];
            if (tree.childCount)
            {
                for (unsigned int child = 0; child < tree.childCount; ++child)
                    pending.push_back(tree.u.firstChildIndex + child);
            }
            else if (tree.materialIndex < cm->numMaterials && tree.u.partitionIndex >= 0 &&
                     tree.u.partitionIndex < cm->partitionCount)
            {
                worldTerrain.push_back(index);
            }
        }
    }
    for (unsigned int i = 0; i < brushes.size(); ++i)
        if (brushes[i])
            worldBrushes.push_back(static_cast<unsigned short>(i));
    indexed = true;
}

int BrushPlanes(const cbrush_t &brush, Plane *planes)
{
    if (brush.numsides > MAX_PLANES - 6 || (brush.numsides && !brush.sides))
        return 0;
    for (int i = 0; i < 6; ++i)
    {
        memset(&planes[i], 0, sizeof(planes[i]));
        const int axis = i / 2;
        planes[i].normal[axis] = (i & 1) ? 1.0f : -1.0f;
        planes[i].dist = (i & 1) ? brush.maxs[axis] : -brush.mins[axis];
    }
    for (unsigned int i = 0; i < brush.numsides; ++i)
    {
        if (!brush.sides[i].plane)
            return 0;
        const cplane_s &source = *brush.sides[i].plane;
        const float length = sqrtf(Dot(source.normal, source.normal));
        if (!(length > 0.000001f))
            return 0;
        Plane &plane = planes[i + 6];
        for (int axis = 0; axis < 3; ++axis)
            plane.normal[axis] = source.normal[axis] / length;
        plane.dist = source.dist / length;
    }
    return 6 + brush.numsides;
}

void EvictOldestBrush()
{
    auto oldest = brushCache.begin();
    for (auto it = brushCache.begin(); it != brushCache.end(); ++it)
        if (it->second.used < oldest->second.used)
            oldest = it;
    cachedPoints -= oldest->second.pointCount;
    brushCache.erase(oldest);
}

const CachedBrush &GetBrush(unsigned int index)
{
    auto found = brushCache.find(index);
    if (found != brushCache.end())
    {
        found->second.used = drawSerial;
        return found->second;
    }

    CachedBrush result;
    result.pointCount = 0;
    result.used = drawSerial;
    const cbrush_t &brush = cm->brushes[index];
    Plane planes[MAX_PLANES];
    const int planeCount = BrushPlanes(brush, planes);
    for (int i = 0; i < planeCount; ++i)
    {
        Point points[MAX_POINTS];
        const int count = BrushFace(planes, planeCount, i, brush.mins, brush.maxs, points);
        if (!count)
            continue;
        Face face;
        face.points.assign(points, points + count);
        memcpy(face.normal, planes[i].normal, sizeof(face.normal));
        memcpy(face.mins, points[0].xyz, sizeof(face.mins));
        memcpy(face.maxs, points[0].xyz, sizeof(face.maxs));
        for (int point = 1; point < count; ++point)
            for (int axis = 0; axis < 3; ++axis)
            {
                face.mins[axis] = (std::min)(face.mins[axis], points[point].xyz[axis]);
                face.maxs[axis] = (std::max)(face.maxs[axis], points[point].xyz[axis]);
            }
        result.pointCount += count;
        result.faces.push_back(face);
    }
    while (!brushCache.empty() &&
           (brushCache.size() >= MAX_CACHED_BRUSHES || cachedPoints + result.pointCount > MAX_CACHED_POINTS))
        EvictOldestBrush();
    cachedPoints += result.pointCount;
    CachedBrush &entry = brushCache[index];
    entry.faces.swap(result.faces);
    entry.pointCount = result.pointCount;
    entry.used = result.used;
    return entry;
}

bool BoundsInView(const refdef_s &view, const float *mins, const float *maxs)
{
    // Conservative AABB tests against the camera's side planes; crossing faces remain visible.
    for (int plane = 0; plane < 5; ++plane)
    {
        float distance = 0.0f;
        float radius = 0.0f;
        for (int axis = 0; axis < 3; ++axis)
        {
            float normal = view.viewaxis[0][axis];
            if (plane)
            {
                const bool horizontal = plane <= 2;
                normal *= horizontal ? view.tanHalfFovX : view.tanHalfFovY;
                normal += (plane & 1 ? 1.0f : -1.0f) * view.viewaxis[horizontal ? 1 : 2][axis];
            }
            distance += (0.5f * (mins[axis] + maxs[axis]) - view.vieworg[axis]) * normal;
            radius += 0.5f * (maxs[axis] - mins[axis]) * fabsf(normal);
        }
        if (distance + radius < 0.0f)
            return false;
    }
    return true;
}

bool HasBounceFace(const cbrush_t &brush)
{
    if (!brush.sides)
        return false;
    for (unsigned int i = 0; i < brush.numsides; ++i)
    {
        const cplane_s *plane = brush.sides[i].plane;
        if (!plane || plane->normal[2] <= 0.0f)
            continue;
        const float length = sqrtf(Dot(plane->normal, plane->normal));
        if (length > 0.000001f && IsBounce(plane->normal[2] / length))
            return true;
    }
    return false;
}

bool CollisionSelected(int contents)
{
    return showCollision->current.integer == 2   ? (contents & PLAYER_SOLID) != 0
           : showCollision->current.integer == 1 ? (contents & CONTENTS_PLAYERCLIP) != 0
                                                 : false;
}

void FaceColor(bool bounce, float normalZ, float *color)
{
    color[0] = 0.1f;
    color[1] = 0.8f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    if (bounce)
    {
        const float steepness = (normalZ - 0.3f) / 0.4f;
        color[0] = steepness;
        color[1] = 1.0f - steepness;
        color[2] = 0.0f;
    }
}

class LineBatch
{
  public:
    explicit LineBatch(int limit) : count(0), remaining(limit), failed(false)
    {
    }

    bool Polygon(const Point *points, int pointCount, const float *color)
    {
        if (failed || pointCount > remaining)
            return false;
        unsigned char packed[4];
        DebugDraw::PackColor(color, packed);
        for (int i = 0; i < pointCount; ++i)
        {
            memcpy(verts[2 * count].xyz, points[i].xyz, sizeof(points[i].xyz));
            memcpy(verts[2 * count + 1].xyz, points[(i + 1) % pointCount].xyz, sizeof(points[i].xyz));
            memcpy(verts[2 * count].color, packed, sizeof(packed));
            memcpy(verts[2 * count + 1].color, packed, sizeof(packed));
            ++count;
            --remaining;
            if (count == 64 && !Flush())
                return false;
        }
        return true;
    }

    bool Flush()
    {
        if (count)
        {
            failed = !DebugDraw::DrawLines(verts, count);
            count = 0;
        }
        return !failed;
    }

    bool Full() const
    {
        return failed || remaining < 3;
    }

  private:
    GfxPointVertex verts[128];
    int count;
    int remaining;
    bool failed;
};

void DrawBrush(unsigned int index, const float *origin, float distanceSquared, LineBatch &batch, bool bounces,
               int &buildsRemaining, const Transform *transform = nullptr)
{
    if (brushCache.find(index) == brushCache.end())
    {
        if (!buildsRemaining)
            return;
        --buildsRemaining;
    }
    const cbrush_t &brush = cm->brushes[index];
    const bool collision = CollisionSelected(brush.contents);
    const CachedBrush &cached = GetBrush(index);
    // Draw highlighted slopes first so a small line budget still shows the bounce candidates.
    for (int pass = 0; pass < 2; ++pass)
        for (size_t i = 0; i < cached.faces.size() && !batch.Full(); ++i)
        {
            const Face &face = cached.faces[i];
            float normal[3], mins[3], maxs[3];
            memcpy(normal, face.normal, sizeof(normal));
            memcpy(mins, face.mins, sizeof(mins));
            memcpy(maxs, face.maxs, sizeof(maxs));
            if (transform)
            {
                TransformNormal(*transform, face.normal, normal);
                TransformBounds(*transform, face.mins, face.maxs, mins, maxs);
            }
            const bool bounce = bounces && IsBounce(normal[2]);
            if (bounce != (pass == 0) || (!collision && !bounce) ||
                BoundsDistanceSquared(origin, mins, maxs) > distanceSquared)
                continue;
            float color[4];
            FaceColor(bounce, normal[2], color);
            if (transform)
            {
                Point points[MAX_POINTS];
                for (size_t point = 0; point < face.points.size(); ++point)
                    TransformPoint(*transform, face.points[point].xyz, points[point].xyz);
                batch.Polygon(points, static_cast<int>(face.points.size()), color);
            }
            else
            {
                batch.Polygon(&face.points[0], static_cast<int>(face.points.size()), color);
            }
        }
}

void DrawTerrain(unsigned int index, const float *origin, float distanceSquared, LineBatch &batch, bool bounces,
                 const Transform *transform = nullptr)
{
    const CollisionAabbTree &tree = cm->aabbTrees[index];
    const CollisionPartition &partition = cm->partitions[tree.u.partitionIndex];
    if (partition.firstTri < 0 || partition.firstTri > cm->triCount - partition.triCount)
        return;
    const bool collision = CollisionSelected(cm->materials[tree.materialIndex].contentFlags);
    for (int i = 0; i < partition.triCount && !batch.Full(); ++i)
    {
        const unsigned short *indices = &cm->triIndices[3 * (partition.firstTri + i)];
        if (indices[0] >= cm->vertCount || indices[1] >= cm->vertCount || indices[2] >= cm->vertCount)
            continue;
        Point points[3];
        float mins[3], maxs[3], a[3], b[3], normal[3];
        for (int vertex = 0; vertex < 3; ++vertex)
        {
            memcpy(points[vertex].xyz, cm->verts[indices[vertex]], sizeof(points[vertex].xyz));
            if (transform)
                TransformPoint(*transform, points[vertex].xyz, points[vertex].xyz);
        }
        for (int axis = 0; axis < 3; ++axis)
        {
            mins[axis] = (std::min)(points[0].xyz[axis], (std::min)(points[1].xyz[axis], points[2].xyz[axis]));
            maxs[axis] = (std::max)(points[0].xyz[axis], (std::max)(points[1].xyz[axis], points[2].xyz[axis]));
            // IW3's collision triangle winding uses (c - a) cross (b - a).
            a[axis] = points[2].xyz[axis] - points[0].xyz[axis];
            b[axis] = points[1].xyz[axis] - points[0].xyz[axis];
        }
        if (BoundsDistanceSquared(origin, mins, maxs) > distanceSquared)
            continue;
        Cross(a, b, normal);
        if (!Normalize(normal))
            continue;
        const bool bounce = bounces && IsBounce(normal[2]);
        if (!collision && !bounce)
            continue;
        float color[4];
        FaceColor(bounce, normal[2], color);
        batch.Polygon(points, 3, color);
    }
}

const CachedModel *GetModel(unsigned int model, int &buildsRemaining)
{
    auto found = modelCache.find(model);
    if (found != modelCache.end())
    {
        found->second.used = drawSerial;
        return &found->second;
    }
    if (!buildsRemaining)
        return nullptr;
    --buildsRemaining;

    CachedModel result;
    result.used = drawSerial;
    const cLeaf_t &leaf = cm->cmodels[model].leaf;
    std::set<unsigned int> nodes, brushes, trees;
    std::vector<unsigned int> pending;
    if (cm->leafbrushNodes && leaf.leafBrushNode > 0)
        pending.push_back(leaf.leafBrushNode);
    while (!pending.empty())
    {
        const unsigned int index = pending.back();
        pending.pop_back();
        if (index >= cm->leafbrushNodesCount || !nodes.insert(index).second)
            continue;
        const cLeafBrushNode_s &node = cm->leafbrushNodes[index];
        if (node.leafBrushCount > 0)
        {
            if (!node.data.leaf.brushes)
                continue;
            for (int i = 0; i < node.leafBrushCount; ++i)
            {
                const unsigned int brush = node.data.leaf.brushes[i];
                if (brush < cm->numBrushes && brushes.insert(brush).second)
                    result.brushes.push_back(static_cast<unsigned short>(brush));
            }
        }
        else
        {
            if (node.leafBrushCount < 0)
                pending.push_back(index + 1);
            for (int child = 0; child < 2; ++child)
                if (node.data.children.childOffset[child])
                    pending.push_back(index + node.data.children.childOffset[child]);
        }
    }
    if (cm->aabbTrees && cm->materials && cm->partitions && cm->verts && cm->triIndices)
        for (unsigned int i = 0; i < leaf.collAabbCount; ++i)
            pending.push_back(leaf.firstCollAabbIndex + i);
    while (!pending.empty())
    {
        const unsigned int index = pending.back();
        pending.pop_back();
        if (index >= static_cast<unsigned int>(cm->aabbTreeCount) || !trees.insert(index).second)
            continue;
        const CollisionAabbTree &tree = cm->aabbTrees[index];
        if (tree.childCount)
        {
            for (unsigned int child = 0; child < tree.childCount; ++child)
                pending.push_back(tree.u.firstChildIndex + child);
        }
        else if (tree.materialIndex < cm->numMaterials && tree.u.partitionIndex >= 0 &&
                 tree.u.partitionIndex < cm->partitionCount)
        {
            result.terrain.push_back(index);
        }
    }
    if (modelCache.size() >= MAX_CACHED_MODELS)
    {
        auto oldest = modelCache.begin();
        for (auto it = modelCache.begin(); it != modelCache.end(); ++it)
            if (it->second.used < oldest->second.used)
                oldest = it;
        modelCache.erase(oldest);
    }
    CachedModel &entry = modelCache[model];
    entry.brushes.swap(result.brushes);
    entry.terrain.swap(result.terrain);
    entry.used = result.used;
    return &entry;
}

void AddModel(unsigned int model, const Transform &transform, const refdef_s &view, float distanceSquared, bool bounces,
              std::vector<ModelInstance> &instances, std::vector<Candidate> &candidates)
{
    // Handle zero is the world. Inline collision is local to its owning entity.
    if (!cm->cmodels || model == 0 || model >= cm->numSubModels)
        return;
    const cmodel_t &collision = cm->cmodels[model];
    const int contents = collision.leaf.brushContents | collision.leaf.terrainContents;
    if (!(contents & PLAYER_SOLID) || (!bounces && !CollisionSelected(contents)))
        return;
    float mins[3], maxs[3];
    TransformBounds(transform, collision.mins, collision.maxs, mins, maxs);
    const float dist = BoundsDistanceSquared(view.vieworg, mins, maxs);
    if (dist > distanceSquared || !BoundsInView(view, mins, maxs))
        return;
    Candidate candidate = {static_cast<unsigned int>(instances.size()), Candidate::MODEL, dist};
    ModelInstance instance = {model, transform};
    instances.push_back(instance);
    candidates.push_back(candidate);
}

void AddEntityModels(int localClientNum, const refdef_s &view, float distanceSquared, bool bounces,
                     std::vector<ModelInstance> &instances, std::vector<Candidate> &candidates)
{
    if (cg_entitiesArray && cg_entitiesArray[localClientNum])
        for (unsigned int i = 0; i < 1024; ++i)
        {
            const centity_s &entity = cg_entitiesArray[localClientNum][i];
            // Match CG_ClipMoveToEntity: disabled brush entities and non-solid snapshots do not collide.
            if (!entity.nextValid || entity.nextState.solid != SOLID_BMODEL ||
                (entity.nextState.lerp.eFlags & ENTITY_COLLISION_DISABLED))
                continue;
            Transform transform;
            AnglesTransform(entity.pose.origin, entity.pose.angles, transform);
            AddModel(entity.nextState.index, transform, view, distanceSquared, bounces, instances, candidates);
        }

    const int type = DYNENT_DRAW_BRUSH;
    if (!cm->dynEntDefList[type] || !cm->dynEntPoseList[type] || !cm->dynEntCollList[type])
        return;
    for (unsigned int i = 0; i < cm->dynEntCount[type]; ++i)
    {
        const DynEntityDef &entity = cm->dynEntDefList[type][i];
        // DynEnt_ClipMoveTraceToBrush additionally checks the native type's clipMove flag.
        if (!cm->dynEntCollList[type][i].sector || entity.type < DYNENT_TYPE_INVALID ||
            entity.type >= DYNENT_TYPE_COUNT || !dynEntProps[entity.type].clipMove || !(entity.contents & PLAYER_SOLID))
            continue;
        const GfxPlacement &pose = cm->dynEntPoseList[type][i].pose;
        Transform transform;
        if (QuaternionTransform(pose.origin, pose.quat, transform))
            AddModel(entity.brushModel, transform, view, distanceSquared, bounces, instances, candidates);
    }
}

void DrawModel(const ModelInstance &instance, const float *origin, float distanceSquared, LineBatch &batch,
               bool bounces, int &buildsRemaining)
{
    const CachedModel *model = GetModel(instance.model, buildsRemaining);
    if (!model)
        return;
    for (size_t i = 0; i < model->brushes.size() && !batch.Full(); ++i)
    {
        const unsigned int brush = model->brushes[i];
        if (cm->brushes[brush].contents & PLAYER_SOLID)
            DrawBrush(brush, origin, distanceSquared, batch, bounces, buildsRemaining, &instance.transform);
    }
    for (size_t i = 0; i < model->terrain.size() && !batch.Full(); ++i)
    {
        const unsigned int tree = model->terrain[i];
        if (cm->materials[cm->aabbTrees[tree].materialIndex].contentFlags & PLAYER_SOLID)
            DrawTerrain(tree, origin, distanceSquared, batch, bounces, &instance.transform);
    }
}
} // namespace

void Collision::OnDvarInit()
{
    showCollision = Dvar_RegisterInt("cm_showCollision", 0, 0, 2, DVAR_FLAG_NONE,
                                     "Draw collision outlines: 0 off, 1 player clip barriers, 2 all player collision");
    showBounces = Dvar_RegisterBool("cm_showBounces", false, DVAR_FLAG_NONE,
                                    "Show candidate bounce slopes and terrain edge contacts");
    drawDistance = Dvar_RegisterInt("cm_showCollisionDist", 1000, 64, 10000, DVAR_FLAG_NONE,
                                    "Maximum distance from the camera to collision outlines");
    maxLines = Dvar_RegisterInt("cm_showCollisionMaxLines", 1024, 64, 4096, DVAR_FLAG_NONE,
                                "Maximum collision outline lines per view; nearest collision draws first");
    bounce_edges::RegisterDvars();
}

void Collision::OnCGInit()
{
    ClearMap();
}

void Collision::OnCGDrawActive(int localClientNum)
{
    if (localClientNum < 0 || localClientNum >= 4 || !showCollision || !showBounces)
        return;
    if (!showCollision->current.integer && !showBounces->current.enabled)
    {
        bounce_edges::Reset();
        return;
    }
    if (!cm->isInUse || !cm->brushes || !cm->leafs || !*cgArray)
    {
        if (indexed)
            ClearMap();
        return;
    }
    if (!indexed || mapBrushes != cm->brushes || mapTrees != cm->aabbTrees || mapModels != cm->cmodels)
        IndexWorld();
    ++drawSerial;

    const bool bounces = showBounces->current.enabled;
    const refdef_s &view = (*cgArray)[localClientNum].refdef;
    const float *origin = view.vieworg;
    const float distance = static_cast<float>(drawDistance->current.integer);
    const float distanceSquared = distance * distance;
    if (bounces)
        bounce_edges::Draw(localClientNum, worldTerrain, distance);
    else
        bounce_edges::Reset();
    std::vector<Candidate> candidates;
    std::vector<ModelInstance> instances;
    for (size_t i = 0; i < worldBrushes.size(); ++i)
    {
        const unsigned int index = worldBrushes[i];
        const cbrush_t &brush = cm->brushes[index];
        if (!(brush.contents & PLAYER_SOLID) ||
            (!CollisionSelected(brush.contents) && (!bounces || !HasBounceFace(brush))))
            continue;
        const float dist = BoundsDistanceSquared(origin, brush.mins, brush.maxs);
        if (dist <= distanceSquared && BoundsInView(view, brush.mins, brush.maxs))
        {
            Candidate candidate = {index, Candidate::BRUSH, dist};
            candidates.push_back(candidate);
        }
    }
    if (cm->materials && cm->partitions && cm->verts && cm->triIndices)
        for (size_t i = 0; i < worldTerrain.size(); ++i)
        {
            const unsigned int index = worldTerrain[i];
            const CollisionAabbTree &tree = cm->aabbTrees[index];
            const int contents = cm->materials[tree.materialIndex].contentFlags;
            if (!(contents & PLAYER_SOLID) || (!CollisionSelected(contents) && !bounces))
                continue;
            float mins[3], maxs[3];
            for (int axis = 0; axis < 3; ++axis)
            {
                mins[axis] = tree.origin[axis] - tree.halfSize[axis];
                maxs[axis] = tree.origin[axis] + tree.halfSize[axis];
            }
            const float dist = BoundsDistanceSquared(origin, mins, maxs);
            if (dist <= distanceSquared && BoundsInView(view, mins, maxs))
            {
                Candidate candidate = {index, Candidate::TERRAIN, dist};
                candidates.push_back(candidate);
            }
        }
    AddEntityModels(localClientNum, view, distanceSquared, bounces, instances, candidates);
    std::sort(candidates.begin(), candidates.end());
    LineBatch batch(maxLines->current.integer);
    // Bound first-time convex clipping work as well as emitted lines. New areas fill in over several frames.
    int buildsRemaining = 32;
    for (size_t i = 0; i < candidates.size() && !batch.Full(); ++i)
    {
        const Candidate &candidate = candidates[i];
        if (candidate.type == Candidate::MODEL)
            DrawModel(instances[candidate.index], origin, distanceSquared, batch, bounces, buildsRemaining);
        else if (candidate.type == Candidate::TERRAIN)
            DrawTerrain(candidate.index, origin, distanceSquared, batch, bounces);
        else
            DrawBrush(candidate.index, origin, distanceSquared, batch, bounces, buildsRemaining);
    }
    batch.Flush();
}

Collision::~Collision()
{
    ClearMap();
}
} // namespace mp
} // namespace iw3
