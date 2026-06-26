#pragma once

#include "pch.h"

class Events : public Module
{
  public:
    Events();
    ~Events();

    const char *get_name() override
    {
        return "Events";
    };

    static void OnDvarInit(const std::function<void()> &callback);
    static void OnCG_DrawActive(const std::function<void()> &callback);
    static void OnCmdInit(const std::function<void()> &callback);
    static void OnDBLinkXAssetPre(
        const std::function<void(iw4::mp_tu6::XAssetType &, iw4::mp_tu6::XAssetHeader *)> &callback);
    static void OnVMShutdown(const std::function<void()> &callback);

  private:
    static std::vector<std::function<void()>> com_initdvars_callbacks;
    static Detour Com_InitDvars_Detour;
    static void Com_InitDvars_Hook();

    static std::vector<std::function<void()>> cg_drawactive_callbacks;
    static Detour CG_DrawActive_Detour;
    static void CG_DrawActive_Hook(int localClientNum);

    static std::vector<std::function<void()>> cmdinit_callbacks;
    static Detour Cmd_Init_Detour;
    static void Cmd_Init_Hook();

    static std::vector<std::function<void(iw4::mp_tu6::XAssetType &, iw4::mp_tu6::XAssetHeader *)>>
        db_linkxasset_pre_callbacks;
    static Detour DB_LinkXAssetEntry1_Detour;
    static iw4::mp_tu6::XAssetEntryPoolEntry *DB_LinkXAssetEntry1_Hook(iw4::mp_tu6::XAssetType type,
                                                                       iw4::mp_tu6::XAssetHeader *header);

    static std::vector<std::function<void()>> vmshutdown_callbacks;
    static Detour Scr_ShutdownSystem_Detour;
    static void Scr_ShutdownSystem_Hook(unsigned __int8 sys);
};
