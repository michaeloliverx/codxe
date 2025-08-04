#include "components/cg.h"
#include "components/clipmap.h"
#include "components/g_client_fields.h"
#include "components/g_scr_main.h"
#include "components/pm.h"
#include "components/scr_parser.h"
#include "main.h"
#include "common.h"

namespace iw4
{
    namespace sp
    {
        void RemoveIdleGunSway()
        {
            // BG_CalculateViewMovement_Angles_Idle
            *(volatile uint32_t *)0x82336C9C = 0x60000000;

            // BG_ComputeAndApplyWeaponMovement_IdleAngles
            *(volatile uint32_t *)0x823362A8 = 0x60000000;
        }

        std::vector<Module *> components;

        void RegisterComponent(Module *module)
        {
            DbgPrint("IW4 SP: Component registered: %s\n", module->get_name());
            components.push_back(module);
        }

        void init()
        {
            DbgPrint("IW4 SP: Registering modules\n");

            RemoveIdleGunSway();

            RegisterComponent(new cg());
            RegisterComponent(new clipmap());
            RegisterComponent(new g_client_fields());
            RegisterComponent(new g_scr_main());
            RegisterComponent(new pm());
            RegisterComponent(new scr_parser());
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
