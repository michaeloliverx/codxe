#include "gsc_client_methods.h"
#include "common.h"

namespace t4
{
    namespace mp
    {
        void PlayerCmd_SprintButtonPressed(scr_entref_t entref)
        {
            if (entref.classnum != 0)
                Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

            auto ent = &g_entities[entref.entnum];
            if (!ent->client)
                Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

            if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
                Scr_Error("Usage: <client> SprintButtonPressed()\n", SCRIPTINSTANCE_SERVER);

            Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_MASK_SPRINT) != 0, SCRIPTINSTANCE_SERVER);
        }

        void PlayerCmd_JumpButtonPressed(scr_entref_t entref)
        {
            if (entref.classnum != 0)
                Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

            auto ent = &g_entities[entref.entnum];
            if (!ent->client)
                Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

            if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
                Scr_Error("Usage: <client> JumpButtonPressed()\n", SCRIPTINSTANCE_SERVER);

            Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_MASK_JUMP) != 0, SCRIPTINSTANCE_SERVER);
        }

        void PlayerCmd_SetVelocity(scr_entref_t entref)
        {
            float velocity[3] = {0};

            if (entref.classnum != 0)
                Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

            auto ent = &g_entities[entref.entnum];
            if (!ent->client)
                Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

            if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
                Scr_Error("Usage: <client> SetVelocity( vec3 )\n", SCRIPTINSTANCE_SERVER);

            Scr_GetVector(0, velocity, SCRIPTINSTANCE_SERVER, -1);

            ent->client->ps.velocity[0] = velocity[0];
            ent->client->ps.velocity[1] = velocity[1];
            ent->client->ps.velocity[2] = velocity[2];
        }

        static struct
        {
            const char *name;
            BuiltinPlayerMethod handler;
        } gsc_player_methods[] = {
            {"sprintbuttonpressed", PlayerCmd_SprintButtonPressed},
            {"jumpbuttonpressed", PlayerCmd_JumpButtonPressed},
            {"setvelocity", PlayerCmd_SetVelocity},
            {nullptr, nullptr} // Terminator
        };

        Detour Player_GetMethod_Detour;

        BuiltinPlayerMethod Player_GetMethod_Hook(const char **pName)
        {
            if (pName != nullptr)
            {
                const auto *func = gsc_player_methods;
                for (; func->name != nullptr; ++func)
                {
                    if (_stricmp(*pName, func->name) == 0)
                        return func->handler;
                }
            }

            return Player_GetMethod_Detour.GetOriginal<decltype(Player_GetMethod)>()(pName);
        }

        void DebugPrintBuiltinMethods()
        {
            for (int i = 0; i < BUILTIN_PLAYER_METHOD_COUNT; ++i)
            {
                const auto &method = builtin_player_methods[i];
                if (method.actionFunc != nullptr)
                {
                    DbgPrint(
                        "method[%d]: actionString: %s actionFunc: %p type: %d\n",
                        i,
                        method.actionString,
                        (void *)method.actionFunc,
                        method.type);
                }
            }
        }

        GSCClientMethods::GSCClientMethods()
        {
            DbgPrint("GSCClientMethods initialized\n");

            Player_GetMethod_Detour = Detour(Player_GetMethod, Player_GetMethod_Hook);
            Player_GetMethod_Detour.Install();
        }

        GSCClientMethods::~GSCClientMethods()
        {
            DbgPrint("GSCClientMethods shutdown\n");
            Player_GetMethod_Detour.Remove();
        }
    }
}
