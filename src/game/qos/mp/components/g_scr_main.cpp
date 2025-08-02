#include "g_scr_main.h"
#include "common.h"

namespace qos
{
    namespace mp
    {
        std::vector<BuiltinFunctionDef *> scr_functions;
        std::vector<BuiltinMethodDef *> scr_methods;

        void Scr_AddFunction(const char *name, BuiltinFunction func, int type)
        {
            BuiltinFunctionDef *newFunc = new BuiltinFunctionDef;
            newFunc->actionString = name;
            newFunc->actionFunc = func;
            newFunc->type = type;
            scr_functions.push_back(newFunc);
        }

        void Scr_AddMethod(const char *name, BuiltinMethod func, int type)
        {
            BuiltinMethodDef *newMethod = new BuiltinMethodDef;
            newMethod->actionString = name;
            newMethod->actionFunc = func;
            newMethod->type = type;
            scr_methods.push_back(newMethod);
        }

        Detour Scr_GetFunction_Detour;

        BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
        {
            if (pName != nullptr)
            {
                for (size_t i = 0; i < scr_functions.size(); ++i)
                {
                    if (std::strcmp(*pName, scr_functions[i]->actionString) == 0)
                    {
                        *type = scr_functions[i]->type;
                        return scr_functions[i]->actionFunc;
                    }
                }
            }

            const BuiltinFunction pFunction = Scr_GetFunction_Detour.GetOriginal<decltype(Scr_GetFunction)>()(pName, type);
            DbgPrint("QOS SP: Scr_GetFunction called for %s, type %d pFunction %p\n", *pName, *type, pFunction);
            return pFunction;
        }

        Detour Scr_GetMethod_Detour;

        BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
        {
            if (pName != nullptr)
            {
                for (size_t i = 0; i < scr_methods.size(); ++i)
                {
                    if (std::strcmp(*pName, scr_methods[i]->actionString) == 0)
                    {
                        *type = scr_methods[i]->type;
                        return scr_methods[i]->actionFunc;
                    }
                }
            }

            const BuiltinMethod pMethod = Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
            DbgPrint("QOS SP: Scr_GetMethod called for %s, type %d pMethod %p\n", *pName, *type, pMethod);
            return pMethod;
        }

        void PlayerCmd_ToggleUFO(scr_entref_t entref)
        {
            if (entref.classnum != 0)
                Scr_ObjectError("not an entity");

            const gentity_s *ent = &g_entities[entref.entnum];
            if (!ent->client)
                Scr_ObjectError(va("entity %i is not a player", entref.entnum));

            ent->client->ufo = !ent->client->ufo;
        }

        void PlayerCmd_GetUFO(scr_entref_t entref)
        {
            if (entref.classnum != 0)
                Scr_ObjectError("not an entity");

            const gentity_s *ent = &g_entities[entref.entnum];
            if (!ent->client)
                Scr_ObjectError(va("entity %i is not a player", entref.entnum));

            Scr_AddInt(ent->client->ufo);
        }

        void PlayerCmd_ToggleGod(scr_entref_t entref)
        {
            if (entref.classnum != 0)
                Scr_ObjectError("not an entity");

            gentity_s *ent = &g_entities[entref.entnum];
            if (!ent->client)
                Scr_ObjectError(va("entity %i is not a player", entref.entnum));

            const int fl_god = 1;

            ent->flags ^= fl_god;
        }

        void DisableBrushCollision()
        {
            for (int i = 0; i < cm->numBrushes; ++i)
            {
                cm->brushes[i].contents &= ~0x10000; // Disable collision for all brushes
            }
        }

        /**
         * Checks if a 3D point is contained within an axis-aligned bounding box
         */
        bool IsPointInBounds(const float mins[3], const float maxs[3], const float point[3])
        {
            return (point[0] >= mins[0] && point[0] <= maxs[0]) &&
                   (point[1] >= mins[1] && point[1] <= maxs[1]) &&
                   (point[2] >= mins[2] && point[2] <= maxs[2]);
        }

        void GScr_DisableBrushCollisionContainingPoint()
        {
            DbgPrint("QOS SP: GScr_DisableBrushCollisionContainingPoint called\n");
            float point[3] = {0};
            Scr_GetVector(0, point);

            DbgPrint("QOS SP: Point to check: (%f, %f, %f)\n", point[0], point[1], point[2]);

            std::vector<int> brushIndices;
            for (int i = 0; i < cm->numBrushes; ++i)
            {
                auto &brush = cm->brushes[i];
                if (brush.contents & 0x10000 /* CONTENTS_PLAYERCLIP */ &&
                    IsPointInBounds(brush.mins, brush.maxs, point))
                {
                    brushIndices.push_back(i);
                }
            }

            DbgPrint("QOS SP: Found %zu brushes containing the point\n", brushIndices.size());

            for (size_t i = 0; i < brushIndices.size(); ++i)
            {
                cm->brushes[brushIndices[i]].contents &= ~0x10000; // Disable collision
            }
        }

        g_scr_main::g_scr_main()
        {
            Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
            Scr_GetFunction_Detour.Install();

            Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
            Scr_GetMethod_Detour.Install();

            // Registering functions
            // Scr_AddFunction("disablebrushcollision", DisableBrushCollision, 0);
            Scr_AddFunction("disablebrushcollisioncontainingpoint", GScr_DisableBrushCollisionContainingPoint, 0);

            // Registering methods
            Scr_AddMethod("togglegod", PlayerCmd_ToggleGod, 0);
            Scr_AddMethod("toggleufo", PlayerCmd_ToggleUFO, 0);
            Scr_AddMethod("getufo", PlayerCmd_GetUFO, 0);
        }

        g_scr_main::~g_scr_main()
        {
            Scr_GetFunction_Detour.Remove();

            Scr_GetMethod_Detour.Remove();
        }
    }
}
