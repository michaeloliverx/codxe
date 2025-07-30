#include "components/clipmap.h"
#include "components/g_client_fields.h"
#include "components/g_client_script_cmd.h"
#include "components/g_scr_main.h"
#include "components/g_scr_mover.h"
#include "components/scr_parser.h"
#include "components/ui.h"
#include "main.h"
#include "common.h"

namespace t4
{
    namespace sp
    {
        std::vector<Module *> components;

        void RegisterComponent(Module *module)
        {
            DbgPrint("T4 SP: Component registered: %s\n", module->get_name());
            components.push_back(module);
        }

        static auto PlayerCmd_finishPlayerDamage = reinterpret_cast<void (*)(scr_entref_t entref)>(0x821C2A38);

        Detour PlayerCmd_finishPlayerDamage_Detour;
        void PlayerCmd_finishPlayerDamage_Detour_Hook(scr_entref_t entref)
        {
            DbgPrint("PlayerCmd_finishPlayerDamage called for entity\n");

            // Call the original function
            PlayerCmd_finishPlayerDamage_Detour.GetOriginal<decltype(PlayerCmd_finishPlayerDamage)>()(entref);
        }

        Detour Scr_GetInt_Detour;
        int Scr_GetInt_Detour_Hook(unsigned int index, scriptInstance_t inst)
        {
            const auto return_address = _ReturnAddress();

            int result = Scr_GetInt_Detour.GetOriginal<decltype(Scr_GetInt)>()(index, inst);

            // DbgPrint("Scr_GetInt called with index: %u, instance: %d, return address: %p result: %d\n", index, inst, return_address, result);

            if (return_address == (void *)0x821C2B60) // PlayerCmd_finishPlayerDamage -> Scr_GetInt(3, 0)
            {
                if (result & 4) // Check if NO_KNOCKBACK flag is set
                {
                    DbgPrint("Scr_GetInt: NO_KNOCKBACK flag detected, clearing it.\n");
                    result &= ~4; // Clear the NO_KNOCKBACK flag
                }
            }

            return result;
        }

        static auto G_Damage = reinterpret_cast<void (*)(int a1,
                                                         int a2,
                                                         int a3,
                                                         int a4,
                                                         int a5,
                                                         int a6,
                                                         int a7,
                                                         int a8,
                                                         int a9,
                                                         int a10,
                                                         int a11,
                                                         int a12)>(0x821C5FA0);

        Detour G_Damage_Detour;
        void G_Damage_Detour_Hook(int a1,
                                  int a2,
                                  int a3,
                                  int a4,
                                  int a5,
                                  int a6,
                                  int a7,
                                  int a8,
                                  int a9,
                                  int a10,
                                  int a11,
                                  int a12)
        {
            // DbgPrint("G_Damage called\n");
            // DbgPrint("a1: %d, a2: %d, a3: %d, a4: %d, a5: %d, a6: %d, a7: %d, a8: %d, a9: %d, a10: %d, a11: %d, a12: %d\n",
            //         a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);

            if (a7 & 4) // Check if NO_KNOCKBACK flag is set
            {
                DbgPrint("G_Damage: NO_KNOCKBACK flag detected, clearing it.\n");
                a7 &= ~4; // Clear the NO_KNOCKBACK flag
            }

            G_Damage_Detour.GetOriginal<decltype(G_Damage)>()(a1,
                                                              a2,
                                                              a3,
                                                              a4,
                                                              a5,
                                                              a6,
                                                              a7,
                                                              a8,
                                                              a9,
                                                              a10,
                                                              a11,
                                                              a12);
        }

        void init()
        {
            DbgPrint("T4 SP: Registering modules\n");
            RegisterComponent(new clipmap());
            RegisterComponent(new g_scr_main()); // Needs to be registered before g_client_script_cmd
            RegisterComponent(new g_client_fields());
            RegisterComponent(new g_client_script_cmd());
            RegisterComponent(new g_scr_mover());
            RegisterComponent(new scr_parser());
            RegisterComponent(new ui());

            Scr_GetInt_Detour = Detour(Scr_GetInt, Scr_GetInt_Detour_Hook);
            Scr_GetInt_Detour.Install();

            // PlayerCmd_finishPlayerDamage_Detour = Detour(PlayerCmd_finishPlayerDamage, PlayerCmd_finishPlayerDamage_Detour_Hook);
            // PlayerCmd_finishPlayerDamage_Detour.Install();

            // // sub_821C2A38 / PlayerCmd_finishPlayerDamage
            // // Patches NO_KNOCKBACK flag check, allows knockback regardless of flags
            // *(volatile uint32_t *)0x821C2CB0 = 0x60000000; // NOP replaces bne

            // *(volatile uint32_t *)0x821C5D2C = 0x60000000; // NOP replaces bne
            // Sleep(50000); // Wait for the game to initialize
            // Dvar_SetBoolByName("sv_cheats", true);
            // Dvar_SetBoolByName("ai_disableSpawn", true);

            // G_Damage_Detour = Detour(G_Damage, G_Damage_Detour_Hook);
            // G_Damage_Detour.Install();
        }

        void shutdown()
        {
            // Clean up in reverse order
            for (auto it = components.rbegin(); it != components.rend(); ++it)
                delete *it;

            components.clear();
        }
    }
}
