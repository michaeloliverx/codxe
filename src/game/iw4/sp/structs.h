#pragma once

#pragma warning(disable : 4480) // nonstandard extension used: specifying underlying type for enum
#pragma warning(disable : 4324) // structure was padded due to __declspec(align())

namespace iw4
{
    namespace sp
    {
        struct playerState_s
        {
            char pad[40];
            float velocity[3];
            char pad1[44144];
        };
        static_assert(sizeof(playerState_s) == 44196, "");
        static_assert(offsetof(playerState_s, velocity) == 40, "");

        struct gclient_s
        {
            playerState_s ps;
            char pad0[176];
            int flags;
            char pad1[640];
        };
        static_assert(sizeof(gclient_s) == 45016, "");
        static_assert(offsetof(gclient_s, ps) == 0x0, "");
        static_assert(offsetof(gclient_s, ps.velocity) == 40, "");
        static_assert(offsetof(gclient_s, flags) == 44372, "");

        struct gentity_s
        {
            char pad0[264];
            gclient_s *client;
            char pad1[44];
            int flags;
            char pad2[308];
        };
        static_assert(sizeof(gentity_s) == 624, "");
        static_assert(offsetof(gentity_s, client) == 264, "");
        static_assert(offsetof(gentity_s, flags) == 312, "");

        struct sentient_s;
        struct actor_s;
        struct Vehicle;
        struct Turret;

        struct level_locals_t
        {
            gclient_s *clients;
            gentity_s *gentities;
            int num_entities;
            gentity_s *firstFreeEnt;
            gentity_s *lastFreeEnt;
            sentient_s *sentients;
            actor_s *actors;
            Vehicle *vehicles;
            Turret *turrets;
            int initializing;
            int clientIsSpawning;
            int maxclients;
        };
        static_assert(offsetof(level_locals_t, clients) == 0x0, "");
        static_assert(offsetof(level_locals_t, gentities) == 0x4, "");
        static_assert(offsetof(level_locals_t, maxclients) == 44, "");

        struct weaponParms
        {
            float forward[3];
            float right[3];
            float up[3];
            float muzzleTrace[3];
            float gunForward[3];
            unsigned int weaponIndex;
            const struct WeaponDef *weapDef;
            const struct WeaponCompleteDef *weapCompleteDef;
        };
        static_assert(sizeof(weaponParms) == 0x48, "");

        struct scr_entref_t
        {
            unsigned __int16 entnum;
            unsigned __int16 classnum;
        };

        typedef void (*BuiltinMethod)(scr_entref_t);

        struct BuiltinMethodDef
        {
            const char *actionString;
            void (*actionFunc)(scr_entref_t);
            int type;
        };

        enum fieldtype_t : __int32
        {
            F_INT = 0x0,
            F_SHORT = 0x1,
            F_BYTE = 0x2,
            F_FLOAT = 0x3,
            F_STRING = 0x4,
            F_CSTRING = 0x5,
            F_VECTOR = 0x6,
            F_ENTITY = 0x7,
            F_ENTHANDLE = 0x8,
            F_ACTOR = 0x9,
            F_SENTIENT = 0xA,
            F_SENTIENTHANDLE = 0xB,
            F_CLIENT = 0xC,
            F_PATHNODE = 0xD,
            F_ANGLES_YAW = 0xE,
            F_MODEL = 0xF,
            F_ACTORGROUP = 0x10,
        };

        struct client_fields_s
        {
            const char *name;
            int ofs;
            fieldtype_t type;
            void (*setter)(gclient_s *, const client_fields_s *);
            void (*getter)(gclient_s *, const client_fields_s *);
        };

        struct Bounds
        {
            float midPoint[3];
            float halfSize[3];
        };

        struct XModel
        {
            const char *name;
        };

        struct cplane_s
        {
            float normal[3];
            float dist;
            unsigned __int8 type;
            unsigned __int8 pad[3];
        };

        struct cStaticModel_s
        {
            XModel *xmodel;
            float origin[3];
            float invScaledAxis[3][3];
            Bounds absBounds;
        };

        struct ClipMaterial
        {
            const char *name;
            int surfaceFlags;
            int contents;
        };

        struct cbrushside_t
        {
            cplane_s *plane;
            unsigned __int16 materialNum;
            unsigned __int8 firstAdjacentSideOffset;
            unsigned __int8 edgeCount;
        };

        struct cNode_t
        {
            cplane_s *plane;
            __int16 children[2];
        };

        struct cLeaf_t
        {
            unsigned __int16 firstCollAabbIndex;
            unsigned __int16 collAabbCount;
            int brushContents;
            int terrainContents;
            Bounds bounds;
            int leafBrushNode;
        };

        struct cLeafBrushNodeLeaf_t
        {
            unsigned __int16 *brushes;
        };

        struct cLeafBrushNodeChildren_t
        {
            float dist;
            float range;
            unsigned __int16 childOffset[2];
        };

        union cLeafBrushNodeData_t
        {
            cLeafBrushNodeLeaf_t leaf;
            cLeafBrushNodeChildren_t children;
        };

        struct cLeafBrushNode_s
        {
            unsigned __int8 axis;
            __int16 leafBrushCount;
            int contents;
            cLeafBrushNodeData_t data;
        };

        struct CollisionBorder
        {
            float distEq[3];
            float zBase;
            float zSlope;
            float start;
            float length;
        };

        struct CollisionPartition
        {
            unsigned __int8 triCount;
            unsigned __int8 borderCount;
            unsigned __int8 firstVertSegment;
            int firstTri;
            CollisionBorder *borders;
        };

        union CollisionAabbTreeIndex
        {
            int firstChildIndex;
            int partitionIndex;
        };

        struct CollisionAabbTree
        {
            float midPoint[3];
            unsigned __int16 materialIndex;
            unsigned __int16 childCount;
            float halfSize[3];
            CollisionAabbTreeIndex u;
        };

        struct cmodel_t
        {
            Bounds bounds;
            float radius;
            cLeaf_t leaf;
        };

        struct cbrush_t
        {
            unsigned __int16 numsides;
            unsigned __int16 glassPieceIndex;
            cbrushside_t *sides;
            unsigned __int8 *baseAdjacentSide;
            __int16 axialMaterialNum[2][3];
            unsigned __int8 firstAdjacentSideOffsets[2][3];
            unsigned __int8 edgeCount[2][3];
        };

        struct TriggerModel
        {
            int contents;
            unsigned __int16 hullCount;
            unsigned __int16 firstHull;
        };

        struct TriggerHull
        {
            Bounds bounds;
            int contents;
            unsigned __int16 slabCount;
            unsigned __int16 firstSlab;
        };

        struct TriggerSlab
        {
            float dir[3];
            float midPoint;
            float halfSize;
        };

        struct MapTriggers
        {
            unsigned int count;
            TriggerModel *models;
            unsigned int hullCount;
            TriggerHull *hulls;
            unsigned int slabCount;
            TriggerSlab *slabs;
        };

        struct __declspec(align(2)) Stage
        {
            const char *name;
            float origin[3];
            unsigned __int16 triggerIndex;
            unsigned __int8 sunPrimaryLightIndex;
        };

        struct __declspec(align(4)) MapEnts
        {
            const char *name;
            char *entityString;
            int numEntityChars;
            MapTriggers trigger;
            Stage *stages;
            unsigned __int8 stageCount;
        };

        struct SModelAabbNode
        {
            Bounds bounds;
            unsigned __int16 firstChild;
            unsigned __int16 childCount;
        };

        enum DynEntityType : __int32
        {
            DYNENT_TYPE_INVALID = 0x0,
            DYNENT_TYPE_CLUTTER = 0x1,
            DYNENT_TYPE_DESTRUCT = 0x2,
            DYNENT_TYPE_COUNT = 0x3,
        };

        struct GfxPlacement
        {
            float quat[4];
            float origin[3];
        };

        struct FxEffectDef;

        struct __declspec(align(4)) PhysPreset
        {
            const char *name;
            int type;
            float mass;
            float bounce;
            float friction;
            float bulletForceScale;
            float explosiveForceScale;
            const char *sndAliasPrefix;
            float piecesSpreadFraction;
            float piecesUpwardVelocity;
            bool tempDefaultToCylinder;
            bool perSurfaceSndAlias;
        };

        struct PhysMass
        {
            float centerOfMass[3];
            float momentsOfInertia[3];
            float productsOfInertia[3];
        };

        struct DynEntityDef
        {
            DynEntityType type;
            GfxPlacement pose;
            XModel *xModel;
            unsigned __int16 brushModel;
            unsigned __int16 physicsBrushModel;
            const FxEffectDef *destroyFx;
            PhysPreset *physPreset;
            int health;
            PhysMass mass;
            int contents;
        };

        struct DynEntityPose
        {
            GfxPlacement pose;
            float radius;
        };

        struct DynEntityClient
        {
            int physObjId;
            unsigned __int16 flags;
            unsigned __int16 lightingHandle;
            int health;
        };

        struct DynEntityColl
        {
            unsigned __int16 sector;
            unsigned __int16 nextEntInSector;
            float linkMins[2];
            float linkMaxs[2];
        };

        struct __declspec(align(64)) clipMap_t
        {
            const char *name;
            int isInUse;
            int planeCount;
            cplane_s *planes;
            unsigned int numStaticModels;
            cStaticModel_s *staticModelList;
            unsigned int numMaterials;
            ClipMaterial *materials;
            unsigned int numBrushSides;
            cbrushside_t *brushsides;
            unsigned int numBrushEdges;
            unsigned __int8 *brushEdges;
            unsigned int numNodes;
            cNode_t *nodes;
            unsigned int numLeafs;
            cLeaf_t *leafs;
            unsigned int leafbrushNodesCount;
            cLeafBrushNode_s *leafbrushNodes;
            unsigned int numLeafBrushes;
            unsigned __int16 *leafbrushes;
            unsigned int numLeafSurfaces;
            unsigned int *leafsurfaces;
            unsigned int vertCount;
            float (*verts)[3];
            int triCount;
            unsigned __int16 *triIndices;
            unsigned __int8 *triEdgeIsWalkable;
            int borderCount;
            CollisionBorder *borders;
            int partitionCount;
            CollisionPartition *partitions;
            int aabbTreeCount;
            CollisionAabbTree *aabbTrees;
            unsigned int numSubModels;
            cmodel_t *cmodels;
            unsigned __int16 numBrushes;
            cbrush_t *brushes;
            Bounds *brushBounds;
            int *brushContents;
            MapEnts *mapEnts;
            unsigned __int16 smodelNodeCount;
            SModelAabbNode *smodelNodes;
            unsigned __int16 dynEntCount[2];
            DynEntityDef *dynEntDefList[2];
            DynEntityPose *dynEntPoseList[2];
            DynEntityClient *dynEntClientList[2];
            DynEntityColl *dynEntCollList[2];
            unsigned int checksum;
        };
        static_assert(sizeof(clipMap_t) == 0x100, "");

    }
}
