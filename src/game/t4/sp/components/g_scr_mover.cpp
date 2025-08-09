#include "g_scr_mover.h"
#include "g_scr_main.h"
#include "common.h"

namespace t4
{
namespace sp
{
void ScriptEntCmd_CloneBrushModelToScriptModel(scr_entref_t entref)
{
    const int script_brushmodel = GScr_AllocString("script_brushmodel");
    const int script_model = GScr_AllocString("script_model");
    const int script_origin = GScr_AllocString("script_origin");
    const int light = GScr_AllocString("light");

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("usage: <scriptModelEnt> CloneBrushModelToScriptModel(<brushModelEnt>)", SCRIPTINSTANCE_SERVER);

    gentity_s *scriptEnt = &g_entities[entref.entnum];
    if (scriptEnt->classname != (unsigned short)script_model)
        Scr_ObjectError("passed entity is not a script_model entity", SCRIPTINSTANCE_SERVER);

    if (scriptEnt->s.eType != ET_SCRIPTMOVER)
        Scr_ObjectError("passed entity type is not 6 (TODO: what is it?)", SCRIPTINSTANCE_SERVER);

    const gentity_s *brushEnt = Scr_GetEntity(0);
    if (brushEnt->classname != (unsigned short)script_brushmodel &&
        brushEnt->classname != (unsigned short)script_model && brushEnt->classname != (unsigned short)script_origin &&
        brushEnt->classname != (unsigned short)light)
        Scr_ParamError(
            0, "brush model entity classname must be one of {script_brushmodel, script_model, script_origin, light}",
            SCRIPTINSTANCE_SERVER);

    if (!brushEnt->s.index)
        Scr_ParamError(0, "brush model entity has no collision model", SCRIPTINSTANCE_SERVER);

    SV_UnlinkEntity(scriptEnt);
    scriptEnt->s.index = brushEnt->s.index;
    int contents = scriptEnt->r.contents;
    SV_SetBrushModel(scriptEnt);
    scriptEnt->r.contents |= contents;
    SV_LinkEntity(scriptEnt);
}

g_scr_mover::g_scr_mover()
{
    Scr_AddMethod("clonebrushmodeltoscriptmodel", ScriptEntCmd_CloneBrushModelToScriptModel, 0);
}

g_scr_mover::~g_scr_mover()
{
}
} // namespace sp
} // namespace t4
