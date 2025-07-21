#include "g_scr_mover.h"
#include "common.h"

namespace t4
{
    namespace sp
    {

        void ScriptEntCmd_CloneBrushModelToScriptModel(scr_entref_t entref)
        {
            const auto script_brushmodel = GScr_AllocString("script_brushmodel");
            const auto script_model = GScr_AllocString("script_model");
            const auto script_origin = GScr_AllocString("script_origin");
            const auto light = GScr_AllocString("light");

            if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
                Scr_Error("usage: <scriptModelEnt> CloneBrushModelToScriptModel(<brushModelEnt>)", SCRIPTINSTANCE_SERVER);

            auto scriptEnt = &g_entities[entref.entnum];
            if (scriptEnt->classname != (unsigned short)script_model)
                Scr_ObjectError("passed entity is not a script_model entity", SCRIPTINSTANCE_SERVER);

            if (scriptEnt->s.eType != ET_SCRIPTMOVER)
                Scr_ObjectError("passed entity type is not 6 (TODO: what is it?)", SCRIPTINSTANCE_SERVER);

            const gentity_s *brushEnt = Scr_GetEntity(0);
            if (brushEnt->classname != (unsigned short)script_brushmodel && brushEnt->classname != (unsigned short)script_model && brushEnt->classname != (unsigned short)script_origin && brushEnt->classname != (unsigned short)light)
                Scr_ParamError(0, "brush model entity classname must be one of {script_brushmodel, script_model, script_origin, light}", SCRIPTINSTANCE_SERVER);

            if (!brushEnt->s.index)
                Scr_ParamError(0, "brush model entity has no collision model", SCRIPTINSTANCE_SERVER);

            SV_UnlinkEntity(scriptEnt);
            scriptEnt->s.index = brushEnt->s.index;
            int contents = scriptEnt->r.contents;
            SV_SetBrushModel(scriptEnt);
            scriptEnt->r.contents |= contents;
            SV_LinkEntity(scriptEnt);
        }

        Detour Scr_GetMethod_Detour;

        BuiltinMethod Scr_GetMethod_Hook(const char **pName, int *type)
        {
            if (pName != nullptr)
            {
                if (std::strcmp(*pName, "clonebrushmodeltoscriptmodel") == 0)
                    return &ScriptEntCmd_CloneBrushModelToScriptModel;
            }

            return Scr_GetMethod_Detour.GetOriginal<decltype(Scr_GetMethod)>()(pName, type);
        }

        g_scr_mover::g_scr_mover()
        {
            Scr_GetMethod_Detour = Detour(Scr_GetMethod, Scr_GetMethod_Hook);
            Scr_GetMethod_Detour.Install();
        }

        g_scr_mover::~g_scr_mover()
        {
            Scr_GetMethod_Detour.Remove();
        }
    }
}
