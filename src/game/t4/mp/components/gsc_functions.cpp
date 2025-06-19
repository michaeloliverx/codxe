#include "gsc_functions.h"
#include "common.h"

namespace t4
{
    namespace mp
    {

        /**
         * Checks if a 3D point is contained within an axis-aligned bounding box
         */
        bool IsPointInBounds(const float mins[3], const float maxs[3], const float point[3])
        {
            return (point[0] >= mins[0] && point[0] <= maxs[0]) &&
                   (point[1] >= mins[1] && point[1] <= maxs[1]) &&
                   (point[2] >= mins[2] && point[2] <= maxs[2]);
        }

        void GSCrGetPlayerclipBrushesContainingPoint()
        {
            float point[3] = {0};
            Scr_GetVector(0, point, SCRIPTINSTANCE_SERVER, -1);

            std::vector<int> brushIndices;
            for (int i = 0; i < cm->numBrushes; ++i)
            {
                auto &brush = cm->brushes[i];
                if (brush.contents & 0x10000 /* CONTENTS_PLAYERCLIP */ && IsPointInBounds(brush.mins, brush.maxs, point))
                    brushIndices.push_back(i);
            }

            Scr_MakeArray(SCRIPTINSTANCE_SERVER);
            for (size_t i = 0; i < brushIndices.size(); ++i)
            {
                Scr_AddInt(brushIndices[i], SCRIPTINSTANCE_SERVER);
                Scr_AddArray(SCRIPTINSTANCE_SERVER);
            }
        }

        void GScr_CloneBrushModelToScriptModel()
        {
            static auto script_brushmodel = GScr_AllocString("script_brushmodel");
            static auto script_model = GScr_AllocString("script_model");
            static auto script_origin = GScr_AllocString("script_origin");
            static auto light = GScr_AllocString("light");

            // CoD4x
            // Common checks.
            if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2)
                Scr_Error("usage: CloneBrushModelToScriptModel(<brushModelEnt>, <scriptModelEnt>)", SCRIPTINSTANCE_SERVER);

            // Object checks.
            gentity_s *scriptEnt = Scr_GetEntity(1);
            if (scriptEnt->classname != script_model)
                Scr_ObjectError("passed entity is not a script_model entity", SCRIPTINSTANCE_SERVER);

            if (scriptEnt->s.eType != 6)
                Scr_ObjectError("passed entity type is not 6 (TODO: what is it?)", SCRIPTINSTANCE_SERVER);

            // Arguments checks.
            gentity_s *brushEnt = Scr_GetEntity(0);
            if (brushEnt->classname != script_brushmodel && brushEnt->classname != script_model && brushEnt->classname != script_origin && brushEnt->classname != light)
                Scr_ParamError(0, "brush model entity classname must be one of {script_brushmodel, script_model, script_origin, light}", SCRIPTINSTANCE_SERVER);

            if (!brushEnt->s.index.brushmodel)
                Scr_ParamError(0, "brush model entity has no collision model", SCRIPTINSTANCE_SERVER);

            // Let's do this...
            SV_UnlinkEntity(scriptEnt);
            scriptEnt->s.index.brushmodel = brushEnt->s.index.brushmodel;
            int contents = scriptEnt->r.contents;
            SV_SetBrushModel(scriptEnt);
            scriptEnt->r.contents |= contents;
            SV_LinkEntity(scriptEnt);
        }

        static struct
        {
            const char *name;
            BuiltinFunction handler;
        } gsc_functions[] = {
            {"getplayerclipbrushescontainingpoint", GSCrGetPlayerclipBrushesContainingPoint},
            {"clonebrushmodeltoscriptmodel", GScr_CloneBrushModelToScriptModel},
            {nullptr, nullptr} // Terminator
        };

        Detour Scr_GetFunction_Detour;

        BuiltinFunction Scr_GetFunction_Hook(const char **pName, int *type)
        {
            if (pName != nullptr)
            {
                const auto *func = gsc_functions;
                for (; func->name != nullptr; ++func)
                {
                    if (_stricmp(*pName, func->name) == 0)
                        return func->handler;
                }
            }
            return Scr_GetFunction_Detour.GetOriginal<decltype(&Scr_GetFunction_Hook)>()(pName, type);
        }

        GSCFunctions::GSCFunctions()
        {
            Scr_GetFunction_Detour = Detour(Scr_GetFunction, Scr_GetFunction_Hook);
            Scr_GetFunction_Detour.Install();
        }

        GSCFunctions::~GSCFunctions()
        {
            Scr_GetFunction_Detour.Remove();
        }
    }
}
