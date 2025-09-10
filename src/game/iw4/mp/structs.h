#pragma once

#pragma warning(disable : 4480) // nonstandard extension used: specifying underlying type for enum
#pragma warning(disable : 4324) // structure was padded due to __declspec(align())

namespace iw4
{
namespace mp
{

union DvarValue
{
    bool enabled;
    int integer;
    unsigned int unsignedInt;
    float value;
    float vector[4];
    const char *string;
    unsigned __int8 color[4];
};

struct DvarLimits_enumeration
{
    int stringCount;
    const char **strings;
};

struct DvarLimits_integer
{
    int min;
    int max;
};

struct DvarLimits_value
{
    float min;
    float max;
};

struct DvarLimits_vector
{
    float min;
    float max;
};

union DvarLimits
{
    DvarLimits_enumeration enumeration;
    DvarLimits_integer integer;
    DvarLimits_value value;
    DvarLimits_vector vector;
};

struct dvar_t
{
    const char *name;
    const char *description;
    unsigned __int16 flags;
    unsigned __int8 type;
    bool modified;
    DvarValue current;
    DvarValue latched;
    DvarValue reset;
    DvarLimits domain;
    dvar_t *hashNext;
};

struct playerState_s
{
    char padding1[0xC];
    uint32_t pm_flags;
    char padding2[0xC];
    float origin[3];
    float velocity[3];
    char padding3[44];
    float delta_angles[3];
    char padding4[160];
    float viewAngles[3];
    char padding5[0x3068];
};
static_assert(sizeof(playerState_s) == 12672, "");
static_assert(offsetof(playerState_s, velocity) == 40, "");
static_assert(offsetof(playerState_s, delta_angles) == 96, "");

struct gclient_s
{
    playerState_s ps;
    char padding1[0x2A0];
    int flags;
    char padding2[0x2DC];
};
static_assert(sizeof(gclient_s) == 14080, "");
static_assert(offsetof(gclient_s, ps) == 0x0, "");
static_assert(offsetof(gclient_s, flags) == 13344, "");

struct gentity_s
{
    char padding1[344];
    gclient_s *client;
    char padding2[0x28];
    int flags;
    char padding3[0xF8];
};
static_assert(sizeof(gentity_s) == 640, "");
static_assert(offsetof(gentity_s, client) == 344, "");

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

struct cmd_function_s
{
    cmd_function_s *next;
    const char *name;
    const char *autoCompleteDir;
    const char *autoCompleteExt;
    void (*function)();
};

struct __declspec(align(4)) usercmd_s
{
    int serverTime;
    int buttons;
    int angles[3];
    unsigned __int16 weapon;
    unsigned __int16 primaryWeaponForAltMode;
    unsigned __int16 offHandIndex;
    char forwardmove;
    char rightmove;
    float meleeChargeYaw;
    unsigned __int8 meleeChargeDist;
    char selectedLoc[2];
    unsigned __int8 selectedLocAngle;
    char remoteControlAngles[2];
};

struct __declspec(align(128)) clSnapshot_t
{
    playerState_s ps;
    int valid;
    int snapFlags;
    int serverTime;
    int messageNum;
    int deltaNum;
    int ping;
    int cmdNum;
    int numEntities;
    int numClients;
    int parseEntitiesIndex;
    int parseClientsIndex;
    int serverCommandNum;
};

enum StanceState : __int32
{
    CL_STANCE_STAND = 0x0,
    CL_STANCE_CROUCH = 0x1,
    CL_STANCE_PRONE = 0x2,
};

struct ClientArchiveData
{
    int serverTime;
    float origin[3];
    float velocity[3];
    int bobCycle;
    int movementDir;
};

struct outPacket_t
{
    int p_cmdNumber;
    int p_serverTime;
    int p_realtime;
};

enum trType_t : __int32
{
    TR_STATIONARY = 0x0,
    TR_INTERPOLATE = 0x1,
    TR_LINEAR = 0x2,
    TR_LINEAR_STOP = 0x3,
    TR_SINE = 0x4,
    TR_GRAVITY = 0x5,
    TR_LOW_GRAVITY = 0x6,
    TR_ACCELERATE = 0x7,
    TR_DECELERATE = 0x8,
    TR_PHYSICS = 0x9,
    TR_FIRST_RAGDOLL = 0xA,
    TR_RAGDOLL = 0xA,
    TR_RAGDOLL_GRAVITY = 0xB,
    TR_RAGDOLL_INTERPOLATE = 0xC,
    TR_LAST_RAGDOLL = 0xC,
    NUM_TRTYPES = 0xD,
};

struct trajectory_t
{
    trType_t trType;
    int trTime;
    int trDuration;
    float trBase[3];
    float trDelta[3];
};

struct LerpEntityStateTurret
{
    float gunAngles[3];
    int lastBarrelRotChangeTime;
    int lastBarrelRotChangeRate;
    int lastHeatChangeLevel;
    int lastHeatChangeTime;
    bool isBarrelRotating;
    bool isOverheat;
    bool isHeatingUp;
    bool isBeingCarried;
};

struct LerpEntityStateLoopFx
{
    float cullDist;
    int period;
};

struct LerpEntityStatePrimaryLight
{
    unsigned __int8 colorAndExp[4];
    float intensity;
    float radius;
    float cosHalfFovOuter;
    float cosHalfFovInner;
};

struct LerpEntityStatePlayer
{
    float leanf;
    int movementDir;
    float torsoPitch;
    float waistPitch;
    unsigned int offhandWeapon;
    int lastSpawnTime;
};

struct LerpEntityStateVehicle
{
    unsigned int indices;
    unsigned int flags;
    float bodyPitch;
    float bodyRoll;
    float steerYaw;
    float gunPitch;
    float gunYaw;
    int playerIndex;
    int pad;
};

struct LerpEntityStatePlane
{
    int ownerNum;
    int enemyIcon;
    int friendIcon;
};

enum MissileFlightMode : __int32
{
    MISSILEFLIGHTMODE_TOP = 0x0,
    MISSILEFLIGHTMODE_DIRECT = 0x1,
    MISSILEFLIGHTMODE_COUNT = 0x2,
};

struct LerpEntityStateMissile
{
    int launchTime;
    MissileFlightMode flightMode;
};

struct LerpEntityStateSoundBlend
{
    float lerp;
};

struct LerpEntityStateBulletHit
{
    float start[3];
};

struct LerpEntityStateEarthquake
{
    float scale;
    float radius;
    int duration;
};

struct LerpEntityStateCustomExplode
{
    int startTime;
};

struct LerpEntityStateExplosion
{
    float innerRadius;
    float outerRadius;
    float magnitude;
};

struct LerpEntityStateExplosionJolt
{
    float innerRadius;
    float outerRadius;
    float impulse[3];
};

struct LerpEntityStatePhysicsJitter
{
    float innerRadius;
    float outerRadius;
    float minDisplacement;
    float maxDisplacement;
};

struct LerpEntityStateRadiusDamage
{
    float range;
    int damageMin;
    int damageMax;
};

struct LerpEntityStateScriptMover
{
    int entToTakeMarksFrom;
    int xModelIndex;
    int animIndex;
    int animTime;
};

struct LerpEntityStateAnonymous
{
    int data[9];
};

union LerpEntityStateTypeUnion
{
    LerpEntityStateTurret turret;
    LerpEntityStateLoopFx loopFx;
    LerpEntityStatePrimaryLight primaryLight;
    LerpEntityStatePlayer player;
    LerpEntityStateVehicle vehicle;
    LerpEntityStatePlane plane;
    LerpEntityStateMissile missile;
    LerpEntityStateSoundBlend soundBlend;
    LerpEntityStateBulletHit bulletHit;
    LerpEntityStateEarthquake earthquake;
    LerpEntityStateCustomExplode customExplode;
    LerpEntityStateExplosion explosion;
    LerpEntityStateExplosionJolt explosionJolt;
    LerpEntityStatePhysicsJitter physicsJitter;
    LerpEntityStateRadiusDamage radiusDamage;
    LerpEntityStateScriptMover scriptMover;
    LerpEntityStateAnonymous anonymous;
};

struct LerpEntityState
{
    int eFlags;
    trajectory_t pos;
    trajectory_t apos;
    LerpEntityStateTypeUnion u;
};

union entityState_s_index
{
    int brushModel;
    int triggerModel;
    int item;
    int xmodel;
    int primaryLight;
};

struct entityState_s_wes
{
    unsigned __int16 weapon;
    unsigned __int16 primaryWeapon;
};

union entityState_s_un1
{
    int eventParm2;
    int hintString;
    int fxId;
    int helicopterStage;
};

union entityState_s_un2
{
    int hintType;
    int vehicleXModel;
    int actorFlags;
    unsigned __int8 weaponModel;
};

struct clientLinkInfo_t
{
    unsigned __int8 flags;
    unsigned __int8 tagName;
    __int16 parentId;
};

struct entityState_s
{
    int number;
    int eType;
    LerpEntityState lerp;
    int time2;
    int otherEntityNum;
    int attackerEntityNum;
    int groundEntityNum;
    int loopSound;
    int surfType;
    entityState_s_index index;
    int clientNum;
    int iHeadIcon;
    int iHeadIconTeam;
    int solid;
    unsigned int eventParm;
    int eventSequence;
    int events[4];
    unsigned int eventParms[4];
    entityState_s_wes wes;
    int legsAnim;
    int torsoAnim;
    entityState_s_un1 un1;
    entityState_s_un2 un2;
    clientLinkInfo_t clientLinkInfo;
    unsigned int partBits[5];
    int clientMask[1];
    unsigned int pad[1];
};

enum team_t : __int32
{
    TEAM_FREE = 0x0,
    TEAM_AXIS = 0x1,
    TEAM_ALLIES = 0x2,
    TEAM_SPECTATOR = 0x3,
    TEAM_NUM_TEAMS = 0x4,
};

struct clientState_s
{
    int clientIndex;
    team_t team;
    int modelindex;
    int dualWielding;
    int riotShieldNext;
    int attachModelIndex[6];
    int attachTagIndex[6];
    char name[32];
    float maxSprintTimeMultiplier;
    int rank;
    int prestige;
    unsigned int perks[2];
    int diveState;
    int voiceConnectivityBits;
    char clanAbbrev[8];
    unsigned int playerCardIcon;
    unsigned int playerCardTitle;
    unsigned int playerCardNameplate;
};

struct __declspec(align(128)) clientActive_t
{
    bool usingAds;
    int timeoutcount;
    clSnapshot_t snap;
    bool alwaysFalse;
    int serverTime;
    int oldServerTime;
    int oldFrameServerTime;
    int serverTimeDelta;
    int oldSnapServerTime;
    int extrapolatedSnapshot;
    int newSnapshots;
    int serverId;
    char mapname[64];
    int parseEntitiesIndex;
    int parseClientsIndex;
    int mouseDx[2];
    int mouseDy[2];
    int mouseIndex;
    bool stanceHeld;
    StanceState stance;
    StanceState stancePosition;
    int stanceTime;
    int cgameUserCmdWeapon;
    int cgameUserCmdOffHandIndex;
    float cgameFOVSensitivityScale;
    float cgameMaxPitchSpeed;
    float cgameMaxYawSpeed;
    float cgameKickAngles[3];
    float cgameOrigin[3];
    float cgameVelocity[3];
    int cgameBobCycle;
    int cgameMovementDir;
    int cgameExtraButtons;
    int cgamePredictedDataServerTime;
    float viewangles[3];
    usercmd_s cmds[128];
    int cmdNumber;
    ClientArchiveData clientArchive[256];
    int clientArchiveIndex;
    int packetBackupCount;
    int packetBackupMask;
    int parseEntitiesCount;
    int parseClientsCount;
    outPacket_t *outPackets;
    clSnapshot_t *snapshots;
    entityState_s *parseEntities;
    clientState_s *parseClients;
    int corruptedTranslationFile;
    char translationVersion[256];
};
static_assert(sizeof(clientActive_t) == 27904, "");

enum connstate_t : __int32
{
    CA_DISCONNECTED = 0x0,
    CA_CINEMATIC = 0x1,
    CA_LOGO = 0x2,
    CA_CONNECTING = 0x3,
    CA_CHALLENGING = 0x4,
    CA_CONNECTED = 0x5,
    CA_SENDINGSTATS = 0x6,
    CA_LOADING = 0x7,
    CA_PRIMED = 0x8,
    CA_ACTIVE = 0x9,
};

struct clientUIActive_t
{
    bool active;
    bool isRunning;
    bool cgameInitialized;       // 0x002 - v7[2] = 1 in retail
    bool cgameInitCalled;        // 0x003 - v7[3] = 1 in retail
    char pad_4[2884];            // 0x004 - padding to reach connectionState
    connstate_t connectionState; // 0xB48 - dword_825A6460[804*a1] in retail
    char pad_B4C[324];           // 0xB4C - padding to reach struct size
};

static_assert(sizeof(clientUIActive_t) == 0xC90, "");                    // 3216 bytes
static_assert(offsetof(clientUIActive_t, connectionState) == 0xB48, ""); // 2888 bytes

struct __declspec(align(128)) cg_s
{
    playerState_s predictedPlayerState;
    char padding[1027200];
};
static_assert(sizeof(cg_s) == 1039872, "");

typedef void (*BuiltinFunction)();
typedef void (*BuiltinMethod)(scr_entref_t);

struct BuiltinFunctionDef
{
    const char *actionString;
    void (*actionFunc)();
    int type;
};

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
    F_CSTRING = 0x4,
    F_STRING = 0x5,
    F_VECTOR = 0x6,
    F_ENTITY = 0x7,
    F_ENTHANDLE = 0x8,
    F_ANGLES_YAW = 0x9,
    F_OBJECT = 0xA,
    F_MODEL = 0xB,
};

struct client_fields_s
{
    const char *name;
    int ofs;
    fieldtype_t type;
    void (*setter)(gclient_s *, const client_fields_s *);
    void (*getter)(gclient_s *, const client_fields_s *);
};

struct Font_s;
struct Material;

struct CachedAssets_t
{
    Material *scrollBarArrowUp;
    Material *scrollBarArrowDown;
    Material *scrollBarArrowLeft;
    Material *scrollBarArrowRight;
    Material *scrollBar;
    Material *scrollBarThumb;
    Material *sliderBar;
    Material *sliderThumb;
    Material *whiteMaterial;
    Material *cursor;
    Material *textDecodeCharacters;
    Material *textDecodeCharactersGlow;
    Font_s *bigFont;
    Font_s *smallFont;
    Font_s *consoleFont;
    Font_s *boldFont;
    Font_s *textFont;
    Font_s *extraBigFont;
    Font_s *objectiveFont;
    Font_s *hudBigFont;
    Font_s *hudSmallFont;
};

struct __declspec(align(4)) sharedUiInfo_t
{
    CachedAssets_t assets;
    bool preventPause;
};

struct ScreenPlacement
{
    float scaleVirtualToReal[2];
    float scaleVirtualToFull[2];
    float scaleRealToVirtual[2];
    float realViewportPosition[2];
    float realViewportSize[2];
    float virtualViewableMin[2];
    float virtualViewableMax[2];
    float realViewableMin[2];
    float realViewableMax[2];
    float virtualAdjustableMin[2];
    float virtualAdjustableMax[2];
    float realAdjustableMin[2];
    float realAdjustableMax[2];
    float subScreenLeft;
};

enum XAssetType : __int32
{
    ASSET_TYPE_PHYSPRESET = 0x0,
    ASSET_TYPE_PHYSCOLLMAP = 0x1,
    ASSET_TYPE_XANIMPARTS = 0x2,
    ASSET_TYPE_XMODEL_SURFS = 0x3,
    ASSET_TYPE_XMODEL = 0x4,
    ASSET_TYPE_MATERIAL = 0x5,
    ASSET_TYPE_PIXELSHADER = 0x6,
    ASSET_TYPE_TECHNIQUE_SET = 0x7,
    ASSET_TYPE_IMAGE = 0x8,
    ASSET_TYPE_SOUND = 0x9,
    ASSET_TYPE_SOUND_CURVE = 0xA,
    ASSET_TYPE_LOADED_SOUND = 0xB,
    ASSET_TYPE_CLIPMAP_SP = 0xC,
    ASSET_TYPE_CLIPMAP_MP = 0xD,
    ASSET_TYPE_COMWORLD = 0xE,
    ASSET_TYPE_GAMEWORLD_SP = 0xF,
    ASSET_TYPE_GAMEWORLD_MP = 0x10,
    ASSET_TYPE_MAP_ENTS = 0x11,
    ASSET_TYPE_FXWORLD = 0x12,
    ASSET_TYPE_GFXWORLD = 0x13,
    ASSET_TYPE_LIGHT_DEF = 0x14,
    ASSET_TYPE_UI_MAP = 0x15,
    ASSET_TYPE_FONT = 0x16,
    ASSET_TYPE_MENULIST = 0x17,
    ASSET_TYPE_MENU = 0x18,
    ASSET_TYPE_LOCALIZE_ENTRY = 0x19,
    ASSET_TYPE_WEAPON = 0x1A,
    ASSET_TYPE_SNDDRIVER_GLOBALS = 0x1B,
    ASSET_TYPE_FX = 0x1C,
    ASSET_TYPE_IMPACT_FX = 0x1D,
    ASSET_TYPE_AITYPE = 0x1E,
    ASSET_TYPE_MPTYPE = 0x1F,
    ASSET_TYPE_CHARACTER = 0x20,
    ASSET_TYPE_XMODELALIAS = 0x21,
    ASSET_TYPE_RAWFILE = 0x22,
    ASSET_TYPE_STRINGTABLE = 0x23,
    ASSET_TYPE_LEADERBOARD = 0x24,
    ASSET_TYPE_STRUCTURED_DATA_DEF = 0x25,
    ASSET_TYPE_TRACER = 0x26,
    ASSET_TYPE_VEHICLE = 0x27,
    ASSET_TYPE_ADDON_MAP_ENTS = 0x28,
    ASSET_TYPE_COUNT = 0x29,
    ASSET_TYPE_STRING = 0x29,
    ASSET_TYPE_ASSETLIST = 0x2A,
};

struct cplane_s;
struct cStaticModel_s;
struct ClipMaterial;
struct cbrushside_t;
struct cNode_t;
struct cLeaf_t;
struct cLeafBrushNode_s;
struct CollisionBorder;
struct CollisionPartition;
struct CollisionAabbTree;
struct cmodel_t;
struct cbrush_t;

struct Bounds
{
    float midPoint[3];
    float halfSize[3];
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

struct SModelAabbNode;
struct DynEntityDef;
struct DynEntityPose;
struct DynEntityClient;
struct DynEntityColl;

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

union XAssetHeader
{
    clipMap_t *clipMap;
    void *data;
};

struct XAsset
{
    XAssetType type;
    XAssetHeader header;
};

struct __declspec(align(4)) XAssetEntry
{
    XAsset asset;
    unsigned __int8 zoneIndex;
    volatile unsigned __int8 inuseMask;
    unsigned __int16 nextHash;
    unsigned __int16 nextOverride;
};

union XAssetEntryPoolEntry
{
    XAssetEntry entry;
    XAssetEntryPoolEntry *next;
};

} // namespace mp
} // namespace iw4
