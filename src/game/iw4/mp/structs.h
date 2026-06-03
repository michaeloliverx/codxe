#pragma once

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

enum DvarFlags : std::uint16_t
{
    DVAR_FLAG_SERVERINFO = 0x10,
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
static_assert(offsetof(dvar_t, current) == 0x08, "");
static_assert(offsetof(dvar_t, latched) == 0x18, "");
static_assert(offsetof(dvar_t, reset) == 0x28, "");
static_assert(offsetof(dvar_t, domain) == 0x38, "");
static_assert(offsetof(dvar_t, hashNext) == 0x40, "");

struct ScreenPlacement;
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

struct WeaponDef;
struct WeaponCompleteDef;

struct weaponParms
{
    float forward[3];
    float right[3];
    float up[3];
    float muzzleTrace[3];
    float gunForward[3];
    unsigned int weaponIndex;
    const WeaponDef *weapDef;
    const WeaponCompleteDef *weapCompleteDef;
};
static_assert(sizeof(weaponParms) == 0x48, "");

struct weaponParms;
struct lockonFireParms;

struct gclient_s;
struct Turret;
struct Vehicle;
struct tagInfo_s;

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
    union
    {
        int brushModel;
        int triggerModel;
        int item;
        int xmodel;
        int primaryLight;
    } index;
    int clientNum;
    int iHeadIcon;
    int iHeadIconTeam;
    int solid;
    unsigned int eventParm;
    int eventSequence;
    int events[4];
    unsigned int eventParms[4];
    struct
    {
        unsigned __int16 weapon;
        unsigned __int16 primaryWeapon;
    } wes;
    int legsAnim;
    int torsoAnim;
    union
    {
        int eventParm2;
        int hintString;
        int fxId;
        int helicopterStage;
    } un1;
    union
    {
        int hintType;
        int vehicleXModel;
        int actorFlags;
        unsigned __int8 weaponModel;
    } un2;
    clientLinkInfo_t clientLinkInfo;
    unsigned int partBits[5];
    int clientMask[1];
    unsigned int pad[1];
};

struct Bounds
{
    float midPoint[3];
    float halfSize[3];
};

struct EntHandle
{
    unsigned __int16 number;
    unsigned __int16 infoIndex;
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

struct __declspec(align(4)) item_ent_t
{
    int ammoCount;
    int clipAmmoCount[2];
    int index;
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

struct mover_ent_t
{
    union
    {
        mover_positions_t pos;
        mover_slidedata_t slide;
    } ___u0;
    mover_positions_t angle;
};

struct corpse_ent_t
{
    int deathAnimStartTime;
};

enum team_t : __int32
{
    TEAM_FREE = 0x0,
    TEAM_AXIS = 0x1,
    TEAM_ALLIES = 0x2,
    TEAM_SPECTATOR = 0x3,
    TEAM_NUM_TEAMS = 0x4,
};

struct missile_fields_grenade
{
    float wobbleCycle;
    float curve;
};

enum MissileStage : __int32
{
    MISSILESTAGE_SOFTLAUNCH = 0x0,
    MISSILESTAGE_ASCENT = 0x1,
    MISSILESTAGE_DESCENT = 0x2,
};

struct missile_fields_nonGrenade
{
    float curvature[3];
    float targetEntOffset[3];
    float targetPos[3];
    float launchOrigin[3];
    MissileStage stage;
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
    union
    {
        missile_fields_grenade grenade;
        missile_fields_nonGrenade nonGrenade;
    } ___u7;
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
    union
    {
        item_ent_t item[2];
        spawner_ent_t spawner;
        trigger_ent_t trigger;
        mover_ent_t mover;
        corpse_ent_t corpse;
        missile_ent_t missile;
        blend_ent_t blend;
    } ___u31;
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

static_assert(sizeof(gentity_s) == 0x280, "");
static_assert(offsetof(gentity_s, client) == 344, "");

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
    unsigned __int8 weaponModel;
    bool needsRechamber[2];
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
    int offHandIndex;
    OffhandClass offhandPrimary;
    OffhandClass offhandSecondary;
    unsigned int weapon;
    unsigned int primaryWeaponForAltMode;
    int weapFlags;
    float fWeaponPosFrac;
    float aimSpreadScale;
    int adsDelayTime;
    int spreadOverride;
    int spreadOverrideState;
    PlayerHandIndex lastWeaponHand;
    GlobalAmmo ammoNotInClip[15];
    ClipAmmo ammoInClip[15];
    int weapLockFlags;
    int weapLockedEntnum;
    float weapLockedPos[3];
    int weaponIdleTime;
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
    unsigned int index;
};

struct ActionSlotParam
{
    ActionSlotParam_SpecifyWeapon specifyWeapon;
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
    int icon;
};

enum he_type_t : __int32
{
    HE_TYPE_FREE = 0x0,
    HE_TYPE_TEXT = 0x1,
    HE_TYPE_VALUE = 0x2,
    HE_TYPE_PLAYERNAME = 0x3,
    HE_TYPE_MAPNAME = 0x4,
    HE_TYPE_GAMETYPE = 0x5,
    HE_TYPE_MATERIAL = 0x6,
    HE_TYPE_TIMER_DOWN = 0x7,
    HE_TYPE_TIMER_UP = 0x8,
    HE_TYPE_TENTHS_TIMER_DOWN = 0x9,
    HE_TYPE_TENTHS_TIMER_UP = 0xA,
    HE_TYPE_CLOCK_DOWN = 0xB,
    HE_TYPE_CLOCK_UP = 0xC,
    HE_TYPE_WAYPOINT = 0xD,
    HE_TYPE_COUNT = 0xE,
};

union hudelem_color_t
{
    struct
    {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    };

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

struct __declspec(align(128)) playerState_s
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
    unsigned int throwbackWeaponIndex;
    int remoteEyesEnt;
    int remoteEyesTagname;
    int remoteControlEnt;
    int foliageSoundTime;
    int gravity;
    float leanf;
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
    int movementDir;
    int eFlags;
    int eventSequence;
    int events[4];
    unsigned int eventParms[4];
    int oldEventSequence;
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
    int locationSelectionInfo;
    SprintState sprintState;
    float holdBreathScale;
    int holdBreathTimer;
    float moveSpeedScaleMultiplier;
    MantleState mantleState;
    PlayerActiveWeaponState weapState[2];
    unsigned int weaponsEquipped[15];
    PlayerEquippedWeaponState weapEquippedData[15];
    PlayerWeaponCommonState weapCommon;
    float meleeChargeYaw;
    int meleeChargeDist;
    int meleeChargeTime;
    unsigned int perks[2];
    unsigned int perkSlots[8];
    ActionSlotType actionSlotType[4];
    ActionSlotParam actionSlotParam[4];
    int weaponHudIconOverrides[6];
    int animScriptedType;
    int shellshockIndex;
    int shellshockTime;
    int shellshockDuration;
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
    struct
    {
        hudelem_s current[31];
        hudelem_s archival[31];
    } hud;
    unsigned int partBits[5];
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
    int predictItemPickup;
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
    int psOffsetTime;
    int hasRadar;
    int isRadarBlocked;
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

enum hintType_t : __int32
{
    HINT_NONE = 0x0,
    HINT_NOICON = 0x1,
    HINT_ACTIVATE = 0x2,
    HINT_HEALTH = 0x3,
    HINT_FRIENDLY = 0x4,
    FIRST_WEAPON_HINT = 0x5,
    LAST_WEAPON_HINT = 0x4B4,
    HINT_NUM_HINTS = 0x4B5,
};

struct gclient_s
{
    playerState_s ps;
    clientSession_t sess;
    int flags;
    int spectatorClient;
    int lastCmdTime;
    int mpviewer;
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
    int weapIdleTime;
    int lastServerTime;
    unsigned int lastWeapon;
    bool previouslyFiring;
    bool previouslyFiringLeftHand;
    bool previouslyUsingNightVision;
    bool previouslySprinting;
    int visionDuration[5];
    char visionName[5][64];
    int lastStand;
    int lastStandTime;
    int hudElemLastAssignedSoundID;
    float lockedTargetOffset[3];
    int attachShieldTagName;
    hintType_t hintForcedType;
    int hintForcedString;
};
static_assert(sizeof(gclient_s) == 0x3700, "");

} // namespace mp
} // namespace iw4
