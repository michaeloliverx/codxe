#include "pch.h"
#include "plugin_manager.h"

template <typename T> std::unique_ptr<Plugin> CreatePlugin()
{
    return make_unique<T>();
}

struct GameInfo
{
    /**
     * Xbox 360 Title ID.
     */
    DWORD titleId;
    /**
     * The XEX build timedatestamp.
     */
    DWORD xexTimestamp;
    /**
     * The original name of the XEX.
     * NOTE: This is not checked and is only for display/dev purposes.
     */
    const char *moduleName;
    /**
     * The value of the in-game `version` dvar.
     * NOTE: This is not checked and is only for display/dev purposes.
     */
    const char *gameVersion;
    /**
     * Human friendly version.
     * NOTE: This is not checked and is only for display/dev purposes.
     */
    const char *friendlyVersion;
    std::unique_ptr<Plugin> (*createPlugin)();
};

const GameInfo GAME_INFO[] = {
    {
        TITLE_ID_QOS,
        0x49E8DEDE, // Fri Apr 17 20:56:14 2009
        "default_mp.xex",
        "",
        "007: Quantum of Solace MP Title Update #2",
        &CreatePlugin<qos::mp::QOS_MP_Plugin>,
    },
    {
        TITLE_ID_QOS,
        0x48D0C2CA, // Wed Sep 17 09:41:46 2008
        "default.xex",
        "",
        "007: Quantum of Solace SP Title Update #2",
        &CreatePlugin<qos::sp::QOS_SP_Plugin>,
    },
    {
        TITLE_ID_IW2,
        0x4456B8A3, // Tue May  2 02:40:51 2006
        "default_mp.xex",
        "",
        "Call of Duty 2 MP Title Update #3",
        &CreatePlugin<iw2::mp::IW2_MP_Plugin>,
    },
    {
        TITLE_ID_IW2,
        0x43E7A218, // Mon Feb  6 19:23:04 2006
        "default.xex",
        "",
        "Call of Duty 2 SP Title Update #3",
        &CreatePlugin<iw2::sp::IW2_SP_Plugin>,
    },
    {
        TITLE_ID_IW3,
        0x4A78A577, // Tue Aug  4 22:17:43 2009
        "default_mp.xex",
        "CoD4 MP 1.4 build 79 nightly Tue Aug 04 2009 01:51:14PM xenon",
        "Call of Duty 4: Modern Warfare MP Title Update #4",
        &CreatePlugin<iw3::mp::IW3_MP_Plugin>,
    },
    {
        TITLE_ID_IW3,
        0x46E1E82F, // Sat Sep  8 01:09:19 2007
        "default.xex",
        "CoD4 1.0 build 472 nightly Fri Sep 07 2007 04:59:49PM xenon",
        "Call of Duty 4: Modern Warfare SP Title Update #4",
        &CreatePlugin<iw3::sp::IW3_SP_Plugin>,
    },
    {
        TITLE_ID_T4,
        0x4AD7C0EE, // Fri Oct 16 01:40:14 2009
        "default_mp.xex",
        "Call of Duty Multiplayer COD_WaW MP build 5.5.51 CL(0) ZQABUILD1 Thu Oct 15 17:39:03 2009 xenon",
        "Call of Duty: World at War MP Title Update #7",
        &CreatePlugin<t4::mp::T4_MP_Plugin>,
    },
    {
        TITLE_ID_T4,
        0x4AD7C0DF, // Fri Oct 16 01:39:59 2009
        "default.xex",
        "",
        "Call of Duty: World at War SP Title Update #7",
        &CreatePlugin<t4::sp::T4_SP_Plugin>,
    },
    {
        TITLE_ID_IW4,
        0x4BE22338, // Thu May 6 03:02:32 2010
        "default_mp.xex",
        "IW4 MP 1.4 build 669 latest Wed May 05 2010 06:55:32PM xenon",
        "Call of Duty: Modern Warfare 2 MP Title Update #6",
        &CreatePlugin<iw4::mp_tu6::IW4_MP_TU6_Plugin>,
    },
    {
        TITLE_ID_IW4,
        0x5B11C269, // Fri Jun 1 23:02:17 2018
        "default_mp.xex",
        "IW4 MP 1.4 build 163842 Tue Feb 08 16:52:00 2011 xenon",
        "Call of Duty: Modern Warfare 2 MP Title Update #9",
        &CreatePlugin<iw4::mp::IW4_MP_Plugin>,
    },
    {
        TITLE_ID_IW4,
        0x5074B056, //  Wed Oct 10 00:16:38 2012
        "default.xex",
        "IW4 1.0 build 125545 Fri Sep 25 22:12:21 2009 xenon",
        "Call of Duty: Modern Warfare 2 SP Title Update #9",
        &CreatePlugin<iw4::sp::IW4_SP_Plugin>,
    },
    {
        TITLE_ID_T5,
        0x4E542876, // Tue Aug 23 23:23:50 2011
        "default_mp.xex",
        "",
        "Call of Duty: Black Ops MP Title Update #11",
        &CreatePlugin<t5::mp::T5_MP_Plugin>,
    },
    {
        TITLE_ID_T5,
        0x4E542875, // Tue Aug 23 23:23:49 2011
        "default.xex",
        "",
        "Call of Duty: Black Ops SP Title Update #11",
        &CreatePlugin<t5::sp::T5_SP_Plugin>,
    },
    {
        TITLE_ID_IW5,
        0x5B10A113, // Fri Jun  1 02:27:47 2018
        "default_mp.xex",
        "IW5 MP 1.8 build 388110 Fri Sep 14 00:04:28 2012 xenon",
        "Call of Duty: Modern Warfare 3 MP Title Update #24",
        &CreatePlugin<iw5::mp::IW5_MP_Plugin>,
    },
};

const GameInfo *FindGameInfo(DWORD title_id, DWORD timestamp)
{
    for (size_t i = 0; i < ARRAYSIZE(GAME_INFO); i++)
    {
        if (GAME_INFO[i].titleId == title_id && GAME_INFO[i].xexTimestamp == timestamp)
        {
            return &GAME_INFO[i];
        }
    }
    return nullptr;
}

PluginManager::PluginManager() : m_current_plugin(nullptr), m_trampoline_pool_baseline(0)
{
}

PluginManager::~PluginManager()
{
    ResetCurrentPlugin();
}

void PluginManager::SetTrampolinePoolBaseline(SIZE_T size)
{
    // The loader hook is process-lifetime state, while game plugin hooks are title-lifetime state.
    // Reset plugin trampolines back to this baseline so unloading a game never overwrites the loader hook trampoline.
    m_trampoline_pool_baseline = size;
}

bool PluginManager::LoadPlugin(const GameInfo *info)
{
    if (info == nullptr)
    {
        return false;
    }

    assert(info->createPlugin != nullptr);
    DbgPrint("[codxe][PluginManager] Loading plugin.\n");

    auto plugin = info->createPlugin();
    if (!plugin)
    {
        DbgPrint("[codxe][PluginManager] Plugin factory returned nullptr.\n");
        return false;
    }

    m_current_plugin = std::move(plugin);
    DbgPrint("[codxe][PluginManager] Plugin loaded.\n");
    return true;
}

void PluginManager::ResetCurrentPlugin()
{
    if (m_current_plugin)
    {
        DbgPrint("[codxe][PluginManager] Unloading plugin.\n");
        m_current_plugin.reset();
        Detour::ResetTrampolinePool(m_trampoline_pool_baseline);
        DbgPrint("[codxe][PluginManager] Plugin unloaded.\n");
    }
}

void PluginManager::OnExecutableLoadStarted()
{
    // Fallback only. Normal cleanup runs from the kernel title-terminate notification before title memory is reset.
    ResetCurrentPlugin();
}

void PluginManager::OnExecutableLoaded(DWORD title_id, DWORD timestamp)
{
    if (title_id == TITLE_ID_DASHBOARD)
    {
        return;
    }

    const GameInfo *info = FindGameInfo(title_id, timestamp);
    if (!info)
    {
        DbgPrint("[codxe][PluginManager] Ignoring unknown executable Title ID:0x%08X TimeDateStamp=0x%08X\n", title_id,
                 timestamp);
        return;
    }

    DbgPrint("[codxe][PluginManager] Game detected: '%s'.\n", info->friendlyVersion);
    xbox::Notify(info->friendlyVersion);
    LoadPlugin(info);
}

void PluginManager::OnTitleTerminate()
{
    ResetCurrentPlugin();
}
