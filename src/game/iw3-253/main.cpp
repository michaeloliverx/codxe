#pragma warning(disable : 4480)

#include <xtl.h>

#include "cg_consolecmds.h"
#include "scr_parser.h"
#include "..\..\detour.h"

extern "C" void DbgPrint(const char *format, ...);

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

    typedef dvar_s *(*Dvar_FindMalleableVar_t)(const char *dvarName);
    Dvar_FindMalleableVar_t Dvar_FindMalleableVar = reinterpret_cast<Dvar_FindMalleableVar_t>(0x821E0780);

    typedef dvar_s *(*Dvar_RegisterBool_t)(const char *dvarName, bool value, unsigned __int16 flags, const char *description);
    Dvar_RegisterBool_t Dvar_RegisterBool = reinterpret_cast<Dvar_RegisterBool_t>(0x821E1718);

    typedef dvar_s *(*Dvar_RegisterInt_t)(const char *dvarName, int value, int min, int max, unsigned __int16 flags, const char *description);
    Dvar_RegisterInt_t Dvar_RegisterInt = reinterpret_cast<Dvar_RegisterInt_t>(0x821E1788);

    typedef void (*PM_FoliageSounds_t)(pmove_t *pm);
    typedef void (*Pmove_t)(pmove_t *pm);
    typedef void (*PmoveSingle_t)(pmove_t *pm);

    PM_FoliageSounds_t PM_FoliageSounds = reinterpret_cast<PM_FoliageSounds_t>(0x820B27F8);
    Pmove_t Pmove = reinterpret_cast<Pmove_t>(0x820B4F48);
    PmoveSingle_t PmoveSingle = reinterpret_cast<PmoveSingle_t>(0x820B4440);

    typedef void (*CG_DrawActive_t)(int localClientNum);
    CG_DrawActive_t CG_DrawActive = reinterpret_cast<CG_DrawActive_t>(0x820CA770);

    typedef int (*R_RegisterFont_t)(const char *name);
    R_RegisterFont_t R_RegisterFont = reinterpret_cast<R_RegisterFont_t>(0x8216EC00);

    typedef void (*R_AddCmdDrawText_t)(const char *text, int maxChars, Font_s *font, double x, double y, double xScale, double yScale, double rotation, const float *color, int style);
    R_AddCmdDrawText_t R_AddCmdDrawText = (R_AddCmdDrawText_t)0x8228F348;

    typedef bool (*Cmd_ExecFromFastFile_t)(int localClientNum, int controllerIndex, const char *filename);
    Cmd_ExecFromFastFile_t Cmd_ExecFromFastFile = reinterpret_cast<Cmd_ExecFromFastFile_t>(0x8223AF40);

    typedef void (*Cbuf_AddText_t)(int localClientNum, const char *text);
    Cbuf_AddText_t Cbuf_AddText = reinterpret_cast<Cbuf_AddText_t>(0x82183458);

    typedef void (*Cbuf_ExecuteBuffer_t)(int localClientNum, const char *buffer);
    Cbuf_ExecuteBuffer_t Cbuf_ExecuteBuffer = reinterpret_cast<Cbuf_ExecuteBuffer_t>(0x821838D0);

    struct scr_entref_t
    {
        unsigned __int16 entnum;
        unsigned __int16 classnum;
    };

    typedef void (*BuiltinMethod)(scr_entref_t);
    typedef BuiltinMethod (*Scr_GetMethod_t)(const char **pName, int *type);
    Scr_GetMethod_t Scr_GetMethod = reinterpret_cast<Scr_GetMethod_t>(0x82167998);

    typedef char *(*Scr_GetString_t)(unsigned int index);
    Scr_GetString_t Scr_GetString = reinterpret_cast<Scr_GetString_t>(0x821AC760);

    typedef void (*PM_UpdateSprint_t)(pmove_t *pm, const pml_t *pml);
    PM_UpdateSprint_t PM_UpdateSprint = reinterpret_cast<PM_UpdateSprint_t>(0x820ADD50);

    ScreenPlacement &scrPlaceFullUnsafe = *reinterpret_cast<ScreenPlacement *>(0x831ACDB8);

    cgMedia_t &cgMedia = *reinterpret_cast<cgMedia_t *>(0x82848250);

    Detour PM_UpdateSprint_Detour;

    void PM_UpdateSprint_Hook(pmove_t *pm, const pml_t *pml)
    {
        DbgPrint("PM_UpdateSprint_Hook\n");
        PM_UpdateSprint_Detour.GetOriginal<decltype(PM_UpdateSprint)>()(pm, pml);

        // Works but prevents bouncing and mantling
        // pm->ps->pm_flags &= ~0x1401Cu;

        // pm->ps->sprintState.lastSprintEnd = 0;
    }

    Detour Pmove_Detour;

    // https://github.com/kejjjjj/iw3sptool/blob/17b669233a1ad086deed867469dc9530b84c20e6/iw3sptool/bg/bg_pmove.cpp#L11
    void Pmove_Hook(pmove_t *pm)
    {
        static dvar_s *pm_fixed_fps_enable = Dvar_FindMalleableVar("pm_fixed_fps_enable");
        static dvar_s *pm_fixed_fps = Dvar_FindMalleableVar("pm_fixed_fps");

        if (!pm_fixed_fps_enable->current.enabled)
            return Pmove_Detour.GetOriginal<decltype(Pmove)>()(pm);

        int msec = 0;
        int cur_msec = 1000 / pm_fixed_fps->current.integer;

        pm->cmd.serverTime = ((pm->cmd.serverTime + (cur_msec < 2 ? 2 : cur_msec) - 1) / cur_msec) * cur_msec;

        int finalTime = pm->cmd.serverTime;

        if (finalTime < pm->ps->commandTime)
        {
            return; // should not happen
        }

        if (finalTime > pm->ps->commandTime + 1000)
            pm->ps->commandTime = finalTime - 1000;
        pm->numtouch = 0;

        while (pm->ps->commandTime != finalTime)
        {
            msec = finalTime - pm->ps->commandTime;

            if (msec > cur_msec)
                msec = cur_msec;

            pm->cmd.serverTime = msec + pm->ps->commandTime;
            PmoveSingle(pm);
            memcpy(&pm->oldcmd, &pm->cmd, sizeof(pm->oldcmd));
        }
    }

    bool g_pm_foliage_ran_this_frame = false;

    void Sys_SnapVector_RoundToNearestEven(float *v)
    {
        // Use __frnd for round-to-nearest-even behavior
        v[0] = (float)__frnd((double)v[0]);
        v[1] = (float)__frnd((double)v[1]);
        v[2] = (float)__frnd((double)v[2]);
    }

    Detour PmoveSingle_Detour;

    void PmoveSingle_Hook(pmove_t *pm)
    {
        DbgPrint("PmoveSingle_Hook\n");
        g_pm_foliage_ran_this_frame = false;
        PmoveSingle_Detour.GetOriginal<decltype(PmoveSingle)>()(pm);
        if (g_pm_foliage_ran_this_frame)
        {
            DbgPrint("Sys_SnapVector_RoundToNearestEven\n");
            Sys_SnapVector_RoundToNearestEven(pm->ps->velocity);
        }

        g_pm_foliage_ran_this_frame = false;
    }

    Detour PM_FoliageSounds_Detour;

    void PM_FoliageSounds_Hook(pmove_t *pm)
    {
        DbgPrint("PM_FoliageSounds_Hook\n");
        PM_FoliageSounds_Detour.GetOriginal<decltype(PM_FoliageSounds)>()(pm);
        g_pm_foliage_ran_this_frame = true;
    }

    void DrawFixedFPS()
    {
        static dvar_s *pm_fixed_fps = Dvar_FindMalleableVar("pm_fixed_fps");

        char buff[16];
        sprintf_s(buff, "%d", pm_fixed_fps->current.integer);

        float color[4] = {1, 1, 1, 1};
        float x = 620 * scrPlaceFullUnsafe.scaleVirtualToFull[0];
        float y = 15 * scrPlaceFullUnsafe.scaleVirtualToFull[1];
        R_AddCmdDrawText(buff, 16, cgMedia.bigDevFont, x, y, 1.0, 1.0, 0.0, color, 0);
    }

    Detour CG_DrawActive_Detour;

    void CG_DrawActive_Hook(int localClientNum)
    {
        static dvar_s *pm_fixed_fps_enable = Dvar_FindMalleableVar("pm_fixed_fps_enable");

        if (pm_fixed_fps_enable->current.enabled)
        {
            DrawFixedFPS();
        }

        CG_DrawActive_Detour.GetOriginal<decltype(CG_DrawActive)>()(localClientNum);
    }

    void GScr_Cbuf_ExecuteBuffer(scr_entref_t entref)
    {
        auto text = Scr_GetString(0);
        Cbuf_ExecuteBuffer(0, text);
    }

    Detour Scr_GetMethod_Detour;

    BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
    {
        if (std::strcmp(*pName, "cbuf_executebuffer") == 0)
            return reinterpret_cast<BuiltinMethod>(&GScr_Cbuf_ExecuteBuffer);

        return Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
    }

    void init()
    {
        Pmove_Detour = Detour(Pmove, Pmove_Hook);
        Pmove_Detour.Install();

        Dvar_RegisterBool("pm_fixed_fps_enable", false, 0, "Enable fixed FPS mode");
        Dvar_RegisterInt("pm_fixed_fps", 250, 0, 1000, 0, "Fixed FPS value");

        init_cg_consolecmds();
        init_scr_parser();

        PmoveSingle_Detour = Detour(PmoveSingle, PmoveSingle_Hook);
        PmoveSingle_Detour.Install();

        PM_FoliageSounds_Detour = Detour(PM_FoliageSounds, PM_FoliageSounds_Hook);
        PM_FoliageSounds_Detour.Install();

        // Nop the function that calls Sys_SnapVector
        uintptr_t start = 0x820B4EB4;
        uintptr_t end = 0x820B4F34;

        for (uintptr_t addr = start; addr <= end; addr += 4)
        {
            *(uint32_t *)addr = 0x60000000; // PowerPC NOP
        }

        CG_DrawActive_Detour = Detour(CG_DrawActive, CG_DrawActive_Hook);
        CG_DrawActive_Detour.Install();

        Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
        Scr_GetMethod_Detour.Install();

        PM_UpdateSprint_Detour = Detour(PM_UpdateSprint, PM_UpdateSprint_Hook);
        PM_UpdateSprint_Detour.Install();

        // Remove read-only protection from DVARs

        //         .text:821E04F4                 bl        _Com_Printf__YAXHPBDZZ # Com_Printf(int,char const *,...)
        // .text:821E04F8                 lwz       r3, 0x4C0+var_40(r1) # cookie
        // .text:821E04FC                 bl        __security_check_cookie
        // .text:821E0500                 addi      r1, r1, 0x4C0
        // .text:821E0504                 b         __restgprlr_27

        // DVARS
        // Patch read-only check to always succeed
        *(unsigned int *)0x821E04E0 = 0x48000028; // b loc_821E0508
        // Patch write protection check to always succeed
        *(unsigned int*)0x821E0510 = 0x48000028; // b loc_821E0538

    }
}
