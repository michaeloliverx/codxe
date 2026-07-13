#pragma once
#include <cstddef>

namespace iw5
{
namespace mp
{

enum MissileStage : __int32
{
    MISSILESTAGE_SOFTLAUNCH = 0x0,
    MISSILESTAGE_ASCENT = 0x1,
    MISSILESTAGE_DESCENT = 0x2,
};

enum team_t : __int32
{
    TEAM_FREE = 0x0,
    TEAM_AXIS = 0x1,
    TEAM_ALLIES = 0x2,
    TEAM_SPECTATOR = 0x3,
    TEAM_NUM_TEAMS = 0x4,
};

enum hintType_t : __int32
{
    HINT_NONE = 0x0,
    HINT_NOICON = 0x1,
    HINT_ACTIVATE = 0x2,
    HINT_HEALTH = 0x3,
    HINT_FRIENDLY = 0x4,
    HINT_WEAPON = 0x5,
    HINT_NUM_HINTS = 0x6,
};

enum $845FE7F0C007B2B115A7B5A33C67D513 : __int32
{
    FL_GODMODE = 0x1,
    FL_DEMI_GODMODE = 0x2,
    FL_NOTARGET = 0x4,
    FL_NO_KNOCKBACK = 0x8,
    FL_NO_RADIUS_DAMAGE = 0x10,
    FL_SUPPORTS_LINKTO = 0x1000,
    FL_NO_AUTO_ANIM_UPDATE = 0x2000,
    FL_GRENADE_TOUCH_DAMAGE = 0x4000,
    FL_STABLE_MISSILES = 0x20000,
    FL_REPEAT_ANIM_UPDATE = 0x40000,
    FL_VEHICLE_TARGET = 0x80000,
    FL_GROUND_ENT = 0x100000,
    FL_CURSOR_HINT = 0x200000,
    FL_MISSILE_ATTRACTOR = 0x800000,
    FL_WEAPON_BEING_GRABBED = 0x1000000,
    FL_DELETE = 0x2000000,
    FL_BOUNCE = 0x4000000,
    FL_MOVER_SLIDE = 0x8000000,
};

enum LocalClientNum_t : __int32
{
    // LOCAL_CLIENT_INVALID = 0xFFFFFFFF,
    LOCAL_CLIENT_0 = 0x0,
    LOCAL_CLIENT_1 = 0x1,
    LOCAL_CLIENT_2 = 0x2,
    LOCAL_CLIENT_3 = 0x3,
    LOCAL_CLIENT_LAST = 0x3,
    LOCAL_CLIENT_COUNT = 0x4,
};

struct Sys_File
{
    void *handle;
};

struct DBFile
{
    Sys_File handle;
    unsigned int memoryBufferSize;
    unsigned __int8 *memoryBuffer;
    char name[64];
};

struct XBlock
{
    unsigned __int8 *data;
    unsigned int size;
};

struct XZoneMemory
{
    XBlock blocks[7];
};

struct XZone
{
    DBFile file;
    int flags;
    int allocType;
    XZoneMemory mem;
};
static_assert(sizeof(XZone) == 0x8C, "");

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
    ASSET_TYPE_CLIPMAP = 0xC,
    ASSET_TYPE_COMWORLD = 0xD,
    ASSET_TYPE_GLASSWORLD = 0xE,
    ASSET_TYPE_PATHDATA = 0xF,
    ASSET_TYPE_VEHICLE_TRACK = 0x10,
    ASSET_TYPE_MAP_ENTS = 0x11,
    ASSET_TYPE_FXWORLD = 0x12,
    ASSET_TYPE_GFXWORLD = 0x13,
    ASSET_TYPE_LIGHT_DEF = 0x14,
    ASSET_TYPE_UI_MAP = 0x15,
    ASSET_TYPE_FONT = 0x16,
    ASSET_TYPE_MENULIST = 0x17,
    ASSET_TYPE_MENU = 0x18,
    ASSET_TYPE_LOCALIZE_ENTRY = 0x19,
    ASSET_TYPE_ATTACHMENT = 0x1A,
    ASSET_TYPE_WEAPON = 0x1B,
    ASSET_TYPE_SNDDRIVER_GLOBALS = 0x1C,
    ASSET_TYPE_FX = 0x1D,
    ASSET_TYPE_IMPACT_FX = 0x1E,
    ASSET_TYPE_SURFACE_FX = 0x1F,
    ASSET_TYPE_AITYPE = 0x20,
    ASSET_TYPE_MPTYPE = 0x21,
    ASSET_TYPE_CHARACTER = 0x22,
    ASSET_TYPE_XMODELALIAS = 0x23,
    ASSET_TYPE_RAWFILE = 0x24,
    ASSET_TYPE_SCRIPTFILE = 0x25,
    ASSET_TYPE_STRINGTABLE = 0x26,
    ASSET_TYPE_LEADERBOARD = 0x27,
    ASSET_TYPE_STRUCTURED_DATA_DEF = 0x28,
    ASSET_TYPE_TRACER = 0x29,
    ASSET_TYPE_VEHICLE = 0x2A,
    ASSET_TYPE_ADDON_MAP_ENTS = 0x2B,
    ASSET_TYPE_COUNT = 0x2C,
    ASSET_TYPE_STRING = 0x2C,
    ASSET_TYPE_ASSETLIST = 0x2D,
};

struct RawFile
{
    const char *name;
    int compressedLen;
    int len;
    const char *buffer;
};

struct ScriptFile
{
    const char *name;
    int compressedLen;
    int len;
    int bytecodeLen;
    const char *buffer;
    unsigned __int8 *bytecode;
};

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

struct ClientTriggerAabbNode
{
    Bounds bounds;
    unsigned __int16 firstChild;
    unsigned __int16 childCount;
};

struct ClientTriggers
{
    MapTriggers trigger;
    unsigned __int16 numClientTriggerNodes;
    ClientTriggerAabbNode *clientTriggerAabbTree;
    unsigned int triggerStringLength;
    char *triggerString;
    unsigned __int16 *triggerStringOffsets;
    unsigned __int8 *triggerType;
    float (*origins)[3];
    float *scriptDelay;
    __int16 *audioTriggers;
};

struct MapEnts
{
    const char *name;
    char *entityString;
    int numEntityChars;
    MapTriggers trigger;
    ClientTriggers clientTrigger;
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

struct ClipInfo;

struct cmodel_t
{
    Bounds bounds;
    float radius;
    ClipInfo *info;
    cLeaf_t leaf;
};

struct GfxBrushModelWritable
{
    Bounds bounds;
};

struct GfxBrushModel
{
    GfxBrushModelWritable writable;
    Bounds bounds;
    float radius;
    unsigned __int16 surfaceCount;
    unsigned __int16 startSurfIndex;
};

struct AddonMapEnts
{
    const char *name;
    char *entityString;
    int numEntityChars;
    MapTriggers trigger;
    ClipInfo *info;
    unsigned int numSubModels;
    cmodel_t *cmodels;
    GfxBrushModel *models;
};

union XAssetHeader
{
    // PhysPreset *physPreset;
    // PhysCollmap *physCollmap;
    // XAnimParts *parts;
    // XModelSurfs *modelSurfs;
    // XModel *model;
    // Material *material;
    // MaterialPixelShader *pixelShader;
    // MaterialVertexShader *vertexShader;
    // MaterialVertexDeclaration *vertexDecl;
    // MaterialTechniqueSet *techniqueSet;
    // GfxImage *image;
    // snd_alias_list_t *sound;
    // SndCurve *sndCurve;
    // LoadedSound *loadSnd;
    // clipMap_t *clipMap;
    // ComWorld *comWorld;
    // GlassWorld *glassWorld;
    // PathData *pathData;
    // VehicleTrack *vehicleTrack;
    MapEnts *mapEnts;
    // FxWorld *fxWorld;
    // GfxWorld *gfxWorld;
    // GfxLightDef *lightDef;
    // Font_s *font;
    // MenuList *menuList;
    // menuDef_t *menu;
    // LocalizeEntry *localize;
    // WeaponAttachment *attachment;
    // WeaponCompleteDef *weapon;
    // SndDriverGlobals *sndDriverGlobals;
    // const FxEffectDef *fx;
    // FxImpactTable *impactFx;
    // SurfaceFxTable *surfaceFx;
    RawFile *rawfile;
    ScriptFile *scriptfile;
    // StringTable *stringTable;
    // LeaderboardDef *leaderboardDef;
    // StructuredDataDefSet *structuredDataDefSet;
    // TracerDef *tracerDef;
    // VehicleDef *vehDef;
    AddonMapEnts *addonMapEnts;
    void *data;
};

struct XAsset
{
    XAssetType type;
    XAssetHeader header;
};

struct XAssetEntry
{
    XAsset asset;
    unsigned __int8 zoneIndex;
    volatile unsigned __int8 inuseMask;
    unsigned __int16 nextHash;
    unsigned __int16 nextOverride;
    unsigned __int16 nextPoolEntry;
};

// Forward declarations
struct gentity_s;
struct Turret;

struct __declspec(align(4)) missileFireParms
{
    gentity_s *target;
    float targetPosOrOffset[3];
    float autoDetonateTime;
    bool lockon;
    bool topFire;
};

struct $E6FD19FCC3453422A2D2E56E256EA75D
{
    unsigned __int32 weaponIdx : 8;
    unsigned __int32 weaponVariation : 4;
    unsigned __int32 weaponScopes : 3;
    unsigned __int32 weaponUnderBarrels : 2;
    unsigned __int32 weaponOthers : 4;
    unsigned __int32 scopeVariation : 3;
    unsigned __int32 padding : 8;
};

union Weapon
{
    $E6FD19FCC3453422A2D2E56E256EA75D __s0;
    unsigned int data;
};

struct WeaponDef;
struct WeaponCompleteDef;

struct weaponParms
{
    float forward[3];
    float right[3];
    float up[3];
    float muzzleTrace[3];
    float gunForward[3];
    Weapon weapon;
    bool isAlternate;
    const WeaponDef *weapDef;
    const WeaponCompleteDef *weapCompleteDef;
};

enum PMem_Source : __int32
{
    PMEM_SOURCE_EXTERNAL = 0x0,
    PMEM_SOURCE_DATABASE = 0x1,
    PMEM_SOURCE_MOVIE = 0x2,
    PMEM_SOURCE_SCRIPT = 0x3,
};

struct PlayerVehicleState
{
    int entity;
    int flags;
    float origin[3];
    float angles[3];
    float velocity[3];
    float angVelocity[3];
    float tilt[2];
    float tiltVelocity[2];
};

struct playerEvents_t
{
    int eventSequence;
    int events[4];
    unsigned int eventParms[4];
    int oldEventSequence;
    int timeADSCameUp;
};

enum ViewLockTypes : __int32
{
    PLAYERVIEWLOCK_NONE = 0x0,
    PLAYERVIEWLOCK_FULL = 0x1,
    PLAYERVIEWLOCK_WEAPONJITTER = 0x2,
    PLAYERVIEWLOCKCOUNT = 0x3,
};

struct SprintState
{
    int sprintButtonUpRequired;
    int sprintDelay;
    int lastSprintStart;
    int lastSprintEnd;
    int sprintStartMaxLength;
};

struct MantleState
{
    float yaw;
    int timer;
    int transIndex;
    int flags;
};

struct PlayerActiveWeaponState
{
    int weapAnim;
    int weaponTime;
    int weaponDelay;
    int weaponRestrictKickTime;
    int weaponState;
    int weapHandFlags;
    unsigned int weaponShotCount;
};

struct PlayerEquippedWeaponState
{
    bool usedBefore;
    bool dualWielding;
    bool inAltMode;
    bool needsRechamber[2];
    int zoomLevelIndex;
};

enum OffhandClass : __int32
{
    OFFHAND_CLASS_NONE = 0x0,
    OFFHAND_CLASS_FRAG_GRENADE = 0x1,
    OFFHAND_CLASS_SMOKE_GRENADE = 0x2,
    OFFHAND_CLASS_FLASH_GRENADE = 0x3,
    OFFHAND_CLASS_THROWINGKNIFE = 0x4,
    OFFHAND_CLASS_OTHER = 0x5,
    OFFHAND_CLASS_COUNT = 0x6,
};

enum PlayerHandIndex : __int32
{
    WEAPON_HAND_RIGHT = 0x0,
    WEAPON_HAND_LEFT = 0x1,
    NUM_WEAPON_HANDS = 0x2,
    WEAPON_HAND_DEFAULT = 0x0,
};

struct GlobalAmmo
{
    int ammoType;
    int ammoCount;
};

struct ClipAmmo
{
    int clipIndex;
    int ammoCount[2];
};

struct PlayerWeaponCommonState
{
    Weapon offHand;
    OffhandClass offhandPrimary;
    OffhandClass offhandSecondary;
    Weapon weapon;
    int weapFlags;
    float fWeaponPosFrac;
    float aimSpreadScale;
    int adsDelayTime;
    int spreadOverride;
    int spreadOverrideState;
    float fAimSpreadMovementScale;
    PlayerHandIndex lastWeaponHand;
    GlobalAmmo ammoNotInClip[15];
    ClipAmmo ammoInClip[15];
    int weapLockFlags;
    int weapLockedEntnum;
    float weapLockedPos[3];
    int weaponIdleTime;
    Weapon lastStowedWeapon;
};

enum ActionSlotType : __int32
{
    ACTIONSLOTTYPE_DONOTHING = 0x0,
    ACTIONSLOTTYPE_SPECIFYWEAPON = 0x1,
    ACTIONSLOTTYPE_ALTWEAPONTOGGLE = 0x2,
    ACTIONSLOTTYPE_NIGHTVISION = 0x3,
    ACTIONSLOTTYPECOUNT = 0x4,
};

struct ActionSlotParam_SpecifyWeapon
{
    Weapon weapon;
};

struct ActionSlotParam
{
    ActionSlotParam_SpecifyWeapon specifyWeapon;
};

enum he_type_t : __int32
{
    HE_TYPE_FREE = 0x0,
    HE_TYPE_TEXT = 0x1,
    HE_TYPE_VALUE = 0x2,
    HE_TYPE_PLAYERNAME = 0x3,
    HE_TYPE_MATERIAL = 0x4,
    HE_TYPE_TIMER_DOWN = 0x5,
    HE_TYPE_TIMER_UP = 0x6,
    HE_TYPE_TIMER_STATIC = 0x7,
    HE_TYPE_TENTHS_TIMER_DOWN = 0x8,
    HE_TYPE_TENTHS_TIMER_UP = 0x9,
    HE_TYPE_TENTHS_TIMER_STATIC = 0xA,
    HE_TYPE_CLOCK_DOWN = 0xB,
    HE_TYPE_CLOCK_UP = 0xC,
    HE_TYPE_WAYPOINT = 0xD,
    HE_TYPE_COUNT = 0xE,
};

struct $0D0CB43DF22755AD856C77DD3F304010
{
    unsigned __int8 r;
    unsigned __int8 g;
    unsigned __int8 b;
    unsigned __int8 a;
};

union hudelem_color_t
{
    $0D0CB43DF22755AD856C77DD3F304010 __s0;
    int rgba;
};

struct hudelem_s
{
    he_type_t type;
    float x;
    float y;
    float z;
    int targetEntNum;
    float fontScale;
    float fromFontScale;
    int fontScaleStartTime;
    int fontScaleTime;
    int font;
    int alignOrg;
    int alignScreen;
    hudelem_color_t color;
    hudelem_color_t fromColor;
    int fadeStartTime;
    int fadeTime;
    int label;
    int width;
    int height;
    int materialIndex;
    int fromWidth;
    int fromHeight;
    int scaleStartTime;
    int scaleTime;
    float fromX;
    float fromY;
    int fromAlignOrg;
    int fromAlignScreen;
    int moveStartTime;
    int moveTime;
    int time;
    int duration;
    float value;
    int text;
    float sort;
    hudelem_color_t glowColor;
    int fxBirthTime;
    int fxLetterTime;
    int fxDecayStartTime;
    int fxDecayDuration;
    int soundID;
    int flags;
};

struct playerState_s_hud
{
    hudelem_s current[31];
    hudelem_s archival[31];
};

enum objectiveState_t : __int32
{
    OBJST_EMPTY = 0x0,
    OBJST_ACTIVE = 0x1,
    OBJST_INVISIBLE = 0x2,
    OBJST_DONE = 0x3,
    OBJST_CURRENT = 0x4,
    OBJST_FAILED = 0x5,
    OBJST_NUMSTATES = 0x6,
};

struct objective_t
{
    objectiveState_t state;
    float origin[3];
    int entNum;
    int teamNum;
    int clientNum;
    int invertVisibilityByClientNum;
    int icon;
};

struct __declspec(align(32)) playerState_s
{
    int commandTime;
    int pm_type;
    int pm_time;
    int pm_flags;
    int otherFlags;
    int linkFlags;
    int bobCycle;
    float origin[3];
    float velocity[3];
    int grenadeTimeLeft;
    int throwbackGrenadeOwner;
    int throwbackGrenadeTimeLeft;
    Weapon throwbackWeapon;
    int remoteEyesEnt;
    int remoteEyesTagname;
    int remoteControlEnt;
    int remoteTurretEnt;
    int foliageSoundTime;
    int gravity;
    int speed;
    float delta_angles[3];
    int groundEntityNum;
    float vLadderVec[3];
    int jumpTime;
    float jumpOriginZ;
    int legsTimer;
    int legsAnim;
    int torsoTimer;
    int torsoAnim;
    int legsAnimDuration;
    int torsoAnimDuration;
    int damageTimer;
    int damageDuration;
    int flinchYawAnim;
    int corpseIndex;
    PlayerVehicleState vehicleState;
    int movementDir;
    int eFlags;
    playerEvents_t pe;
    int unpredictableEventSequence;
    int unpredictableEventSequenceOld;
    int unpredictableEvents[4];
    unsigned int unpredictableEventParms[4];
    int clientNum;
    int viewmodelIndex;
    float viewangles[3];
    int viewHeightTarget;
    float viewHeightCurrent;
    int viewHeightLerpTime;
    int viewHeightLerpTarget;
    int viewHeightLerpDown;
    float viewAngleClampBase[2];
    float viewAngleClampRange[2];
    int damageEvent;
    int damageYaw;
    int damagePitch;
    int damageCount;
    int damageFlags;
    int stats[4];
    float proneDirection;
    float proneDirectionPitch;
    float proneTorsoPitch;
    ViewLockTypes viewlocked;
    int viewlocked_entNum;
    float linkAngles[3];
    float linkWeaponAngles[3];
    int linkWeaponEnt;
    int loopSound;
    int cursorHint;
    int cursorHintString;
    int cursorHintEntIndex;
    int cursorHintDualWield;
    int iCompassPlayerInfo;
    int radarEnabled;
    int radarBlocked;
    int radarMode;
    int radarStrength;
    int radarShowEnemyDirection;
    int locationSelectionInfo;
    SprintState sprintState;
    float holdBreathScale;
    int holdBreathTimer;
    float moveSpeedScaleMultiplier;
    MantleState mantleState;
    PlayerActiveWeaponState weapState[2];
    Weapon weaponsEquipped[15];
    PlayerEquippedWeaponState weapEquippedData[15];
    PlayerWeaponCommonState weapCommon;
    int meleeChargeDist;
    int meleeChargeTime;
    int meleeChargeEnt;
    unsigned int airburstMarkDistance;
    unsigned int perks[2];
    unsigned int perkSlots[9];
    ActionSlotType actionSlotType[4];
    ActionSlotParam actionSlotParam[4];
    int weaponHudIconOverrides[6];
    int animScriptedType;
    int shellshockIndex;
    int shellshockTime;
    int shellshockDuration;
    float viewKickScale;
    float dofNearStart;
    float dofNearEnd;
    float dofFarStart;
    float dofFarEnd;
    float dofNearBlur;
    float dofFarBlur;
    float dofViewmodelStart;
    float dofViewmodelEnd;
    objective_t objective[32];
    int deltaTime;
    int killCamEntity;
    int killCamLookAtEntity;
    int killCamClientNum;
    playerState_s_hud hud;
    unsigned int partBits[6];
    int recoilScale;
    int diveDirection;
    int stunTime;
};

enum sessionState_t : __int32
{
    SESS_STATE_PLAYING = 0x0,
    SESS_STATE_DEAD = 0x1,
    SESS_STATE_SPECTATOR = 0x2,
    SESS_STATE_INTERMISSION = 0x3,
};

enum clientConnected_t : __int32
{
    CON_DISCONNECTED = 0x0,
    CON_CONNECTING = 0x1,
    CON_CONNECTED = 0x2,
};

struct __declspec(align(2)) usercmd_s
{
    int serverTime;
    int buttons;
    int angles[3];
    Weapon weapon;
    Weapon offHand;
    char forwardmove;
    char rightmove;
    unsigned __int16 airburstMarkDistance;
    unsigned __int16 meleeChargeEnt;
    unsigned __int8 meleeChargeDist;
    char selectedLoc[2];
    unsigned __int8 selectedLocAngle;
    char remoteControlAngles[2];
    char remoteControlMove[3];
};
static_assert(sizeof(usercmd_s) == 44, "");

struct playerTeamState_t
{
    int location;
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
    unsigned int use_elite_clan_tag;
    unsigned int use_elite_clan_title;
    char elite_clan_title_text[24];
    int elite_clan_title_bg;
    char elite_clan_tag_text[8];
    int prestige_black_ops;
    int rank_black_ops;
    int prestige_mw2;
    int rank_mw2;
    int prestige_world_at_war;
    int rank_world_at_war;
    int prestige_mw;
    int rank_mw;
};

struct clientSession_t
{
    sessionState_t sessionState;
    int forceSpectatorClient;
    int killCamEntity;
    int killCamLookAtEntity;
    int status_icon;
    int archiveTime;
    int score;
    int deaths;
    int kills;
    int assists;
    unsigned __int16 scriptPersId;
    clientConnected_t connected;
    usercmd_s cmd;
    usercmd_s oldcmd;
    int localClient;
    char newnetname[32];
    int maxHealth;
    int enterTime;
    playerTeamState_t teamState;
    int voteCount;
    int teamVoteCount;
    float moveSpeedScaleMultiplier;
    int viewmodelIndex;
    int noSpectate;
    int teamInfo;
    clientState_s cs;
    char _pad[4]; // TODO: Remove this when the cause of the misalignment is found
    int psOffsetTime;
    int hasRadar;
    int isRadarBlocked;
    int radarStrength;
    int radarShowEnemyDirection;
    int radarMode;
    int weaponHudIconOverrides[6];
    unsigned int unusableEntFlags[64];
    float spectateDefaultPos[3];
    float spectateDefaultAngles[3];
};

struct viewClamp
{
    float start[2];
    float current[2];
    float goal[2];
};

struct viewClampState
{
    viewClamp min;
    viewClamp max;
    float accelTime;
    float decelTime;
    float totalTime;
    float startTime;
};

struct EntHandle
{
    unsigned __int16 number;
    unsigned __int16 infoIndex;
};

// NOTE: There is an alignment/padding issue here somewhere
// Fields that are used should be verified with static_assert on their offsets against the latest version of the game
struct __declspec(align(128)) gclient_s
{
    playerState_s ps;
    clientSession_t sess;
    int flags;
    int spectatorClient;
    int lastCmdTime;
    int buttons;
    int oldbuttons;
    int latched_buttons;
    int buttonsSinceLastFrame;
    float oldOrigin[3];
    float fGunPitch;
    float fGunYaw;
    int damage_blood;
    int damage_stun;
    float damage_from[3];
    int damage_fromWorld;
    int accurateCount;
    int accuracy_shots;
    int accuracy_hits;
    int inactivityTime;
    int inactivityWarning;
    int lastVoiceTime;
    int switchTeamTime;
    float currentAimSpreadScale;
    float prevLinkedInvQuat[4];
    bool prevLinkAnglesSet;
    bool link_rotationMovesEyePos;
    bool link_doCollision;
    bool link_useTagAnglesForViewAngles;
    bool link_useBaseAnglesForViewClamp;
    float linkAnglesFrac;
    viewClampState link_viewClamp;
    gentity_s *persistantPowerup;
    int portalID;
    int dropWeaponTime;
    int sniperRifleFiredTime;
    float sniperRifleMuzzleYaw;
    int PCSpecialPickedUpCount;
    EntHandle useHoldEntity;
    int useHoldTime;
    int useButtonDone;
    int iLastCompassPlayerInfoEnt;
    int compassPingTime;
    int damageTime;
    float v_dmg_roll;
    float v_dmg_pitch;
    float baseAngles[3];
    float baseOrigin[3];
    float swayViewAngles[3];
    float swayOffset[3];
    float swayAngles[3];
    float recoilAngles[3];
    float recoilSpeed[3];
    float fLastIdleFactor;
    int lastServerTime;
    Weapon lastWeapon;
    bool lastWeaponAltStatus;
    bool previouslyFiring;
    bool previouslyFiringLeftHand;
    bool previouslyUsingNightVision;
    bool previouslySprinting;
    int visionDuration[6];
    char visionName[6][64];
    int lastStand;
    int lastStandTime;
    int hudElemLastAssignedSoundID;
    float lockedTargetOffset[3];
    unsigned __int16 attachShieldTagName;
    hintType_t hintForcedType;
    int hintForcedString;
};
static_assert(sizeof(gclient_s) == 14720, "");

static_assert(offsetof(gclient_s, ps.stunTime) == 13028, "");
static_assert(offsetof(gclient_s, sess.sessionState) == 13056, "");
static_assert(offsetof(gclient_s, sess.spectateDefaultAngles) == 13808, "");
static_assert(offsetof(gclient_s, flags) == 13820, "");

enum netadrtype_t : __int32
{
    NA_BOT = 0x0,
    NA_BAD = 0x1,
    NA_LOOPBACK = 0x2,
    NA_BROADCAST = 0x3,
    NA_IP = 0x4,
};

enum netsrc_t : __int32
{
    NS_CLIENT1 = 0x0,
    NS_CLIENT2 = 0x1,
    NS_CLIENT3 = 0x2,
    NS_CLIENT4 = 0x3,
    NS_MAXCLIENTS = 0x4,
    NS_SERVER = 0x4,
    NS_PACKET = 0x5,
    NS_INVALID_NETSRC = 0x6,
};

struct netadr_t
{
    netadrtype_t type;
    unsigned __int8 ip[4];
    unsigned __int16 port;
    netsrc_t localNetID;
};

struct netProfilePacket_t
{
    int iTime;
    int iSize;
    int bFragment;
};

struct netProfileStream_t
{
    netProfilePacket_t packets[60];
    int iCurrPacket;
    int iBytesPerSecond;
    int iLastBPSCalcTime;
    int iCountedPackets;
    int iCountedFragments;
    int iFragmentPercentage;
    int iLargestPacket;
    int iSmallestPacket;
};

struct netProfileInfo_t
{
    netProfileStream_t send;
    netProfileStream_t recieve;
};

struct netchan_t
{
    int outgoingSequence;
    netsrc_t sock;
    int dropped;
    int incomingSequence;
    netadr_t remoteAddress;
    int fragmentSequence;
    int fragmentLength;
    unsigned __int8 *fragmentBuffer;
    int fragmentBufferSize;
    int unsentFragments;
    int unsentFragmentStart;
    int unsentLength;
    unsigned __int8 *unsentBuffer;
    int unsentBufferSize;
    netProfileInfo_t prof;
};

struct clientHeader_t
{
    int state;
    int sendAsActive;
    int deltaMessage;
    int rateDelayed;
    int hasAckedBaselineData;
    int hugeSnapshotSent;
    netchan_t netchan;
    float predictedOrigin[3];
    int predictedOriginServerTime;
    int migrationState;
    float predictedVehicleOrigin[3];
    int predictedVehicleServerTime;
};

struct client_t
{
    clientHeader_t header;
    const char *dropReason;
    char *userinfo;
    char pad01[134580];
    gentity_s *gentity;
    char pad1[67772];
    int bIsSplitscreenClient;
    char pad2[13830];
    uint16_t scriptId;
    char pad3[4];
    int bIsTestClient;
    char pad4[211088];
};
static_assert(sizeof(client_t) == 428928, "");
static_assert(offsetof(client_t, header) == 0, "");
static_assert(offsetof(client_t, userinfo) == 1636, "");
static_assert(offsetof(client_t, gentity) == 136220, "");
static_assert(offsetof(client_t, bIsSplitscreenClient) == 203996, "");
static_assert(offsetof(client_t, scriptId) == 217830, "");
static_assert(offsetof(client_t, bIsTestClient) == 217836, "");

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

struct __declspec(align(2)) LerpEntityStateTurret
{
    float gunAngles[3];
    int lastBarrelRotChangeTime;
    unsigned __int16 lastBarrelRotChangeRate;
    unsigned __int16 lastHeatChangeLevel;
    int lastHeatChangeTime;
    bool isBarrelRotating;
    bool isOverheat;
    bool isHeatingUp;
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
    int movementDir;
    float torsoPitch;
    float waistPitch;
    Weapon offhandWeapon;
    Weapon stowedWeapon;
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
    int data[7];
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

struct StaticEntityStateVehicle
{
    int playerIndex;
};

struct StaticEntityStateTurret
{
    bool isBeingCarried;
};

struct StaticEntityStateAnonymous
{
    int data[1];
};

union StaticEntityStateTypeUnion
{
    StaticEntityStateVehicle vehicle;
    StaticEntityStateTurret turret;
    StaticEntityStateAnonymous anonymous;
};

union entityState_s_index
{
    int brushModel;
    int triggerModel;
    int xmodel;
    int primaryLight;
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
    unsigned __int16 vehicleXModel;
    int actorFlags;
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
    StaticEntityStateTypeUnion staticState;
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
    Weapon weapon;
    int inAltWeaponMode;
    int legsAnim;
    int torsoAnim;
    entityState_s_un1 un1;
    entityState_s_un2 un2;
    clientLinkInfo_t clientLinkInfo;
    unsigned int partBits[6];
    int clientMask[1];
};

struct entityShared_t
{
    unsigned __int8 isLinked;
    unsigned __int8 modelType;
    unsigned __int8 svFlags;
    unsigned __int8 isInUse;
    Bounds box;
    int contents;
    Bounds absBox;
    float currentOrigin[3];
    float currentAngles[3];
    EntHandle ownerNum;
    int eventTime;
};

struct corpse_ent_t
{
    int deathAnimStartTime;
};

struct missile_fields_grenade
{
    float wobbleCycle;
    float curve;
};

struct missile_fields_nonGrenade
{
    float curvature[3];
    float targetEntOffset[3];
    float targetPos[3];
    float launchOrigin[3];
    MissileStage stage;
};

union $21ADA81E94310923948D9663431E4310
{
    missile_fields_grenade grenade;
    missile_fields_nonGrenade nonGrenade;
};

struct missile_ent_t
{
    float time;
    int timeOfBirth;
    float travelDist;
    float surfaceNormal[3];
    team_t team;
    int flags;
    int antilagTimeOffset;
    $21ADA81E94310923948D9663431E4310 ___u7;
};

struct blend_ent_t
{
    float pos[3];
    float vel[3];
    float viewQuat[4];
    bool changed;
    float accelTime;
    float decelTime;
    float startTime;
    float totalTime;
};

struct __declspec(align(4)) item_ent_t
{
    int ammoCount;
    int clipAmmoCount[2];
    Weapon weapon;
    bool dualWieldItem;
};

struct spawner_ent_t
{
    int team;
    int timestamp;
    int index;
};

struct __declspec(align(4)) trigger_ent_t
{
    int threshold;
    int accumulate;
    int timestamp;
    int singleUserEntIndex;
    bool requireLookAt;
};

struct mover_positions_t
{
    float decelTime;
    float speed;
    float midTime;
    float pos1[3];
    float pos2[3];
    float pos3[3];
};

struct mover_slidedata_t
{
    Bounds bounds;
    float velocity[3];
};

union $E5E7A8FB561C1217389792297346F5D5
{
    mover_positions_t pos;
    mover_slidedata_t slide;
};

struct mover_ent_t
{
    $E5E7A8FB561C1217389792297346F5D5 ___u0;
    mover_positions_t angle;
};

union $11CCB19A78CF1B6CBECC71799583BBA8
{
    item_ent_t item[2];
    spawner_ent_t spawner;
    trigger_ent_t trigger;
    mover_ent_t mover;
    corpse_ent_t corpse;
    missile_ent_t missile;
    blend_ent_t blend;
};

struct tagInfo_s
{
    gentity_s *parent;
    gentity_s *next;
    unsigned __int16 name;
    bool blendToParent;
    int index;
    float axis[4][3];
    float parentInvAxis[4][3];
};
struct Vehicle;

struct gentity_s
{
    entityState_s s;
    entityShared_t r;
    gclient_s *client;
    Turret *turret;
    Vehicle *vehicle;
    int physObjId;
    unsigned __int16 model;
    unsigned __int8 physicsObject;
    unsigned __int8 takedamage;
    unsigned __int8 active;
    unsigned __int8 handler;
    unsigned __int8 team;
    bool freeAfterEvent;
    __int16 padding_short;
    unsigned __int16 classname;
    unsigned __int16 script_classname;
    unsigned __int16 script_linkName;
    unsigned __int16 target;
    unsigned __int16 targetname;
    unsigned int attachIgnoreCollision;
    int spawnflags;
    int flags;
    int eventTime;
    int clipmask;
    int processedFrame;
    EntHandle parent;
    int nextthink;
    int health;
    int maxHealth;
    int damage;
    int count;
    $11CCB19A78CF1B6CBECC71799583BBA8 ___u31;
    EntHandle missileTargetEnt;
    EntHandle remoteControlledOwner;
    tagInfo_s *tagInfo;
    gentity_s *tagChildren;
    unsigned __int16 attachModelNames[19];
    unsigned __int16 attachTagNames[19];
    int useCount;
    gentity_s *nextFree;
    int birthTime;
    int padding[3];
};
static_assert(sizeof(gentity_s) == 640, "");
static_assert(offsetof(gentity_s, client) == 344, "");
static_assert(offsetof(gentity_s, flags) == 388, "");

struct SpawnVar
{
    bool spawnVarsValid;
    int numSpawnVars;
    unsigned int spawnVarsKey[64];
    char *spawnVarsValue[64];
    int numSpawnVarChars;
    char spawnVarChars[2048];
};

struct trigger_info_t
{
    unsigned __int16 entnum;
    unsigned __int16 otherEntnum;
    int useCount;
    int otherUseCount;
};

struct com_parse_mark_t
{
    int lines;
    const char *text;
    int ungetToken;
    int backup_lines;
    const char *backup_text;
};

struct cached_tag_mat_t
{
    int time;
    int entnum;
    unsigned __int16 name;
    float tagMat[4][3];
};

struct level_locals_t
{
    gclient_s *clients;
    gentity_s *gentities;
    int num_entities;
    gentity_s *firstFreeEnt;
    gentity_s *lastFreeEnt;
    Turret *turrets;
    int initializing;
    int clientIsSpawning;
    objective_t objectives[32];
    int maxclients;
    int framenum;
    int time;
    int previousTime;
    int frametime;
    int startTime;
    int teamScores[4];
    int bUpdateScoresForIntermission;
    bool teamHasRadar[4];
    bool teamRadarBlocked[4];
    int teamRadarStrength[4];
    int manualNameChange;
    int numConnectedClients;
    int sortedClients[18];
    SpawnVar spawnVar;
    int savepersist;
    EntHandle droppedWeaponCue[32];
    float fFogOpaqueDist;
    float fFogOpaqueDistSqrd;
    int currentPlayerClone;
    trigger_info_t pendingTriggerList[256];
    trigger_info_t currentTriggerList[256];
    int pendingTriggerListSize;
    int currentTriggerListSize;
    int finished;
    int bPlayerIgnoreRadiusDamage;
    int bPlayerIgnoreRadiusDamageLatched;
    int registerWeapons;
    int bRegisterItems;
    int currentEntityThink;
    void *openScriptIOFileHandles[1];
    char *openScriptIOFileBuffers[1];
    com_parse_mark_t currentScriptIOLineMark[1];
    cached_tag_mat_t cachedTagMat;
    int scriptPrintChannel;
    float compassMapUpperLeft[2];
    float compassMapWorldSize[2];
    float compassNorth[2];
    Vehicle *vehicles;
    int hudElemLastAssignedSoundID;
};
static_assert(sizeof(level_locals_t) == 10348, "");

struct scr_entref_t
{
    unsigned __int16 entnum;
    unsigned __int16 classnum;
};

typedef void (*BuiltinFunction)();
struct BuiltinFunctionDef
{
    unsigned int name;
    void (*actionFunc)();
    int type;
};

typedef void (*BuiltinMethod)(scr_entref_t);
struct BuiltinMethodDef
{
    unsigned int name;
    void (*actionFunc)(scr_entref_t);
    int type;
};

struct cplane_s
{
    float normal[3];
    float dist;
    unsigned __int8 type;
    unsigned __int8 pad[3];
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

struct ClipInfo
{
    int planeCount;
    cplane_s *planes;
    unsigned int numMaterials;
    ClipMaterial *materials;
    unsigned int numBrushSides;
    cbrushside_t *brushsides;
    unsigned int numBrushEdges;
    unsigned __int8 *brushEdges;
    unsigned int leafbrushNodesCount;
    cLeafBrushNode_s *leafbrushNodes;
    unsigned int numLeafBrushes;
    unsigned __int16 *leafbrushes;
    unsigned __int16 numBrushes;
    cbrush_t *brushes;
    Bounds *brushBounds;
    int *brushContents;
};

struct __declspec(align(32)) clipMap_t
{
    const char *name;
    int isInUse;
    ClipInfo info;
    //   ClipInfo *pInfo;
    //   unsigned int numStaticModels;
    //   cStaticModel_s *staticModelList;
    //   unsigned int numNodes;
    //   cNode_t *nodes;
    //   unsigned int numLeafs;
    //   cLeaf_t *leafs;
    //   unsigned int vertCount;
    //   float (*verts)[3];
    //   int triCount;
    //   unsigned __int16 *triIndices;
    //   unsigned __int8 *triEdgeIsWalkable;
    //   int borderCount;
    //   CollisionBorder *borders;
    //   int partitionCount;
    //   CollisionPartition *partitions;
    //   int aabbTreeCount;
    //   CollisionAabbTree *aabbTrees;
    //   unsigned int numSubModels;
    //   cmodel_t *cmodels;
    //   MapEnts *mapEnts;
    //   Stage *stages;
    //   unsigned __int8 stageCount;
    //   MapTriggers stageTrigger;
    //   unsigned __int16 smodelNodeCount;
    //   SModelAabbNode *smodelNodes;
    //   unsigned __int16 dynEntCount[2];
    //   DynEntityDef *dynEntDefList[2];
    //   DynEntityPose *dynEntPoseList[2];
    //   DynEntityClient *dynEntClientList[2];
    //   DynEntityColl *dynEntCollList[2];
    //   unsigned int checksum;
};

struct pmove_t;
struct pml_t;

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

struct dvar_t
{
    const char *name;
    unsigned __int16 flags;
    unsigned __int8 type;
    bool modified;
    DvarValue current;
    DvarValue latched;
    DvarValue reset;
    DvarLimits domain;
    dvar_t *hashNext;
};

struct Material;
struct Font_s;

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

struct __declspec(align(8)) sharedUiInfo_t
{
    CachedAssets_t assets;
    // ...more
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

// Custom Structs

struct SpawnBotOptions
{
    int entNum;
    int level;
    int prestige;
};

// Function typedefs

typedef void (*CG_GameMessage_t)(LocalClientNum_t localClientNum, const char *msg);

typedef Font_s *(*CL_RegisterFont_t)(const char *fontName, int imageTrack);

typedef XAssetEntry *(*DB_FindXAssetEntry_t)(XAssetType type, const char *name);
typedef XAssetHeader *(*DB_FindXAssetHeader_t)(XAssetType type, const char *name, int allowCreateDefault);
typedef XAssetEntry *(*DB_LinkXAssetEntry_t)(XAssetType type, XAssetHeader *header);
typedef const char *(*DB_GetXAssetName_t)(const XAsset *asset);
typedef bool (*DB_IsXAssetDefault_t)(XAssetType type, const char *name);

typedef dvar_t *(*Dvar_FindMalleableVar_t)(const char *dvarName);

typedef void (*Jump_Start_t)(pmove_t *pm, pml_t *pml, double height);

typedef unsigned __int8 *(*PMem_AllocFromSource_NoDebug_t)(unsigned int size, unsigned int alignment, unsigned int type,
                                                           PMem_Source source);

typedef void (*Scr_AddInt_t)(int value);
typedef void (*Scr_ErrorInternal_t)();
typedef gentity_s *(*GetEntity_t)(scr_entref_t entref);
typedef int (*Scr_GetInt_t)(unsigned int index);
typedef unsigned int (*Scr_GetMethod_t)(const char **pName, int *type);
typedef const char *(*Scr_GetString_t)(unsigned int index);
typedef int (*Sl_GetString_t)(const char *string, int user);
typedef unsigned int (*Scr_GetNumParam_t)();

typedef const ScreenPlacement *(*ScrPlace_GetActivePlacement_t)(const LocalClientNum_t localClientNum);

typedef void (*PlayerCmd_GetViewmodel_t)(scr_entref_t entref);
typedef void (*PlayerCmd_UFO_t)(scr_entref_t *entref);

typedef void (*UI_DrawBuildNumber_t)(LocalClientNum_t localClientNum);

typedef void (*UI_DrawText_t)(const ScreenPlacement *scrPlace, const char *text, int maxChars, Font_s *font, double x,
                              double y, int horzAlign, int vertAlign, double scale, const float *color, int style);

typedef gentity_s *(*Weapon_RocketLauncher_Fire_t)(gentity_s *ent, const Weapon *weapon, double spread, weaponParms *wp,
                                                   weaponParms *gunVel, missileFireParms *fireParms,
                                                   missileFireParms *magicBullet, bool a8);

typedef void (*SV_DropClient_t)(client_t *cl, const char *reason, bool tellThem);

} // namespace mp
} // namespace iw5