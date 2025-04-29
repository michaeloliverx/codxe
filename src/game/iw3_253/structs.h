#pragma warning(disable : 4480)

namespace iw3_253
{
    struct __declspec(align(4)) usercmd_s
    {
        int serverTime;
        int buttons;
        unsigned __int8 weapon;
        unsigned __int8 offHandIndex;
        int angles[3];
        char forwardmove;
        char rightmove;
        float meleeChargeYaw;
        unsigned __int8 meleeChargeDist;
    };

    enum OffhandSecondaryClass : __int32
    {
        PLAYER_OFFHAND_SECONDARY_SMOKE = 0x0,
        PLAYER_OFFHAND_SECONDARY_FLASH = 0x1,
        PLAYER_OFFHAND_SECONDARIES_TOTAL = 0x2,
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

    struct playerState_s
    {
        int commandTime;
        int pm_type;
        int bobCycle;
        int pm_flags;
        int weapFlags;
        int otherFlags;
        int pm_time;
        float origin[3];
        float velocity[3];
        float oldVelocity[2];
        int weaponTime;
        int weaponDelay;
        int grenadeTimeLeft;
        int throwBackGrenadeOwner;
        int throwBackGrenadeTimeLeft;
        int weaponRestrictKickTime;
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
        int movementDir;
        int eFlags;
        int eventSequence;
        int events[4];
        unsigned int eventParms[4];
        int oldEventSequence;
        int clientNum;
        int offHandIndex;
        OffhandSecondaryClass offhandSecondary;
        unsigned int weapon;
        int weaponstate;
        unsigned int weaponShotCount;
        float fWeaponPosFrac;
        int adsDelayTime;
        int spreadOverride;
        int spreadOverrideState;
        int viewmodelIndex;
        float viewangles[3];
        int viewHeightTarget;
        float viewHeightCurrent;
        int viewHeightLerpTime;
        int viewHeightLerpTarget;
        int viewHeightLerpDown;
        float viewHeightLerpPosAdj;
        float viewAngleClampBase[2];
        float viewAngleClampRange[2];
        int damageEvent;
        int damageYaw;
        int damagePitch;
        int damageCount;
        int stats[5];
        int ammo[128];
        int ammoclip[128];
        unsigned int weapons[4];
        unsigned int weaponold[4];
        unsigned int weaponrechamber[4];
        float proneDirection;
        float proneDirectionPitch;
        float proneTorsoPitch;
        ViewLockTypes viewlocked;
        int viewlocked_entNum;
        int cursorHint;
        int cursorHintString;
        int cursorHintEntIndex;
        int iCompassPlayerInfo;
        int radarEnabled;
        int locationSelectionInfo;
        SprintState sprintState;
        float fTorsoPitch;
        float fWaistPitch;
        float holdBreathScale;
        int holdBreathTimer;
        MantleState mantleState;
        float meleeChargeYaw;
        int meleeChargeDist;
        ActionSlotType actionSlotType[4];
        ActionSlotParam actionSlotParam[4];
        int entityEventSequence;
        int weapAnim;
        float aimSpreadScale;
        int shellshockIndex;
        int shellshockTime;
        int shellshockDuration;
        float dofNearStart;
        float dofNearEnd;
        float dofFarStart;
        float dofFarEnd;
        float dofNearBlur;
        float dofFarBlur;
        objective_t objective[16];
        int deltaTime;
        // playerState_s::<unnamed_tag>hud;
    };

    struct __declspec(align(4)) pmove_t
    {
        playerState_s *ps;
        usercmd_s cmd;
        usercmd_s oldcmd;
        int tracemask;
        int numtouch;
        int touchents[32];
        float mins[3];
        float maxs[3];
        float xyspeed;
        int proneChange;
        float maxSprintTimeMultiplier;
        bool mantleStarted;
        float mantleEndPos[3];
        int mantleDuration;
        unsigned __int8 handler;
    };

    struct DvarValueStringBuf
    {
        const char *pad;
        char string[12];
    };

    union DvarValue
    {
        bool enabled;
        int integer;
        unsigned int unsignedInt;
        float value;
        float vector[4];
        const char *string;
        DvarValueStringBuf stringBuf;
        unsigned __int8 color[4];
    };

    union DvarLimits
    {
        struct
        {
            int stringCount;
            const char **strings;
        } enumeration;

        struct
        {
            int min;
            int max;
        } integer;

        struct
        {
            float min;
            float max;
        } value;

        struct
        {
            float min;
            float max;
        } vector;
    };

    struct dvar_s
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
        dvar_s *next;
        dvar_s *hashNext;
    };

    struct ScreenPlacement
    {
        float scaleVirtualToReal[2];
        float scaleVirtualToFull[2];
        float scaleRealToVirtual[2];
        float virtualScreenOffsetX;
        float virtualViewableMin[2];
        float virtualViewableMax[2];
        float realViewportSize[2];
        float realViewableMin[2];
        float realViewableMax[2];
        float subScreenLeft;
    };

    struct Material;
    struct Font_s;

    struct cgMedia_t
    {
        Material *whiteMaterial;
        Material *teamStatusBar;
        Material *balloonMaterial;
        Material *connectionMaterial;
        Material *youInKillCamMaterial;
        Material *tracerMaterial;
        Material *laserMaterial;
        Material *laserLightMaterial;
        Material *lagometerMaterial;
        Material *hintMaterials[133];
        Material *stanceMaterials[4];
        Material *objectiveMaterials[1];
        Material *friendMaterials[2];
        Material *damageMaterial;
        Material *mantleHint;
        Font_s *smallDevFont;
        Font_s *bigDevFont;
        // snd_alias_list_t *landDmgSound;
        // snd_alias_list_t *grenadeExplodeSound[29];
        // snd_alias_list_t *rocketExplodeSound[29];
        // snd_alias_list_t *bulletHitSmallSound[29];
        // snd_alias_list_t *bulletHitLargeSound[29];
        // snd_alias_list_t *bulletHitAPSound[29];
        // snd_alias_list_t *shotgunHitSound[29];
        // snd_alias_list_t *stepRunSound[29];
        // snd_alias_list_t *stepRunSoundPlayer[29];
        // snd_alias_list_t *stepWalkSound[29];
        // snd_alias_list_t *stepWalkSoundPlayer[29];
        // snd_alias_list_t *stepProneSound[29];
        // snd_alias_list_t *stepProneSoundPlayer[29];
        // snd_alias_list_t *landSound[29];
        // snd_alias_list_t *landSoundPlayer[29];
        // snd_alias_list_t *runningEquipmentSound;
        // snd_alias_list_t *runningEquipmentSoundPlayer;
        // snd_alias_list_t *walkingEquipmentSound;
        // snd_alias_list_t *walkingEquipmentSoundPlayer;
        // snd_alias_list_t *foliageMovement;
        // snd_alias_list_t *bulletWhizby;
        // snd_alias_list_t *meleeSwingLarge;
        // snd_alias_list_t *meleeSwingSmall;
        // snd_alias_list_t *meleeHit;
        // snd_alias_list_t *meleeHitOther;
        // snd_alias_list_t *nightVisionOn;
        // snd_alias_list_t *nightVisionOff;
        // snd_alias_list_t *playerSprintGasp;
        // snd_alias_list_t *playerHeartBeatSound;
        // snd_alias_list_t *playerBreathInSound;
        // snd_alias_list_t *playerBreathOutSound;
        // snd_alias_list_t *playerBreathGaspSound;
        // snd_alias_list_t *playerSwapOffhand;
        // snd_alias_list_t *physCollisionSound[50][29];
        // Material *compassping_friendlyfiring;
        // Material *compassping_friendlyyelling;
        // Material *compassping_enemy;
        // Material *compassping_enemyfiring;
        // Material *compassping_enemyyelling;
        // Material *compassping_grenade;
        // Material *compassping_explosion;
        // Material *compass_radarline;
        // Material *compass_helicopter_friendly;
        // Material *compass_helicopter_enemy;
        // Material *compass_plane_friendly;
        // Material *compass_plane_enemy;
        // Material *grenadeIconFrag;
        // Material *grenadeIconFlash;
        // Material *grenadeIconThrowBack;
        // Material *grenadePointer;
        // FxImpactTable *fx;
        // const FxEffectDef *fxNoBloodFleshHit;
        // const FxEffectDef *heliDustEffect;
        // const FxEffectDef *heliWaterEffect;
        // const FxEffectDef *helicopterLightSmoke;
        // const FxEffectDef *helicopterHeavySmoke;
        // const FxEffectDef *helicopterOnFire;
        // const FxEffectDef *jetAfterburner;
        // const FxEffectDef *fxVehicleTireDust;
        // XModel *nightVisionGoggles;
        // Material *nightVisionOverlay;
        // Material *hudIconNVG;
        // Material *hudDpadArrow;
        // Material *ammoCounterBullet;
        // Material *ammoCounterBeltBullet;
        // Material *ammoCounterRifleBullet;
        // Material *ammoCounterRocket;
        // Material *ammoCounterShotgunShell;
    };

    enum TraceHitType : __int32
    {
        TRACE_HITTYPE_NONE = 0x0,
        TRACE_HITTYPE_ENTITY = 0x1,
        TRACE_HITTYPE_DYNENT_MODEL = 0x2,
        TRACE_HITTYPE_DYNENT_BRUSH = 0x3,
    };

    struct __declspec(align(2)) trace_t
    {
        float fraction;
        float normal[3];
        int surfaceFlags;
        int contents;
        const char *material;
        TraceHitType hitType;
        unsigned __int16 hitId;
        unsigned __int16 modelIndex;
        unsigned __int16 partName;
        unsigned __int16 partGroup;
        bool allsolid;
        bool startsolid;
        bool walkable;
    };

    struct pml_t
    {
        float forward[3];
        float right[3];
        float up[3];
        float frametime;
        int msec;
        int walking;
        int groundPlane;
        int almostGroundPlane;
        trace_t groundTrace;
        float impactSpeed;
        float previous_origin[3];
        float previous_velocity[3];
    };

}