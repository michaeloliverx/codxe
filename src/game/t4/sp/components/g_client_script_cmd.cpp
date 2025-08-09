#include "g_client_script_cmd.h"
#include "g_scr_main.h"
#include "common.h"

namespace t4
{
namespace sp
{
client_t *VM_GetClientForEntRef(scr_entref_t entref)
{
    return &(*reinterpret_cast<client_t **>(0x839EC08C))[entref.entnum];
}

void PlayerCmd_JumpButtonPressed(scr_entref_t entref)
{
    if (entref.classnum != 0)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

    const gentity_s *ent = &g_entities[entref.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> JumpButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_GOSTAND) != 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_secondaryOffhandButtonPressed(scr_entref_t entref)
{
    if (entref.classnum != 0)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

    const gentity_s *ent = &g_entities[entref.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> SecondaryOffhandButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_SMOKE) != 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_SprintButtonPressed(scr_entref_t entref)
{
    if (entref.classnum != 0)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

    const gentity_s *ent = &g_entities[entref.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> SprintButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_SPRINT) != 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_MoveForwardButtonPressed(scr_entref_t entref)
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> MoveForwardButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    const client_t *cl = VM_GetClientForEntRef(entref);

    if (!cl)
        Scr_ObjectError("not a client\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(cl->lastUsercmd.forwardmove > 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_MoveBackButtonPressed(scr_entref_t entref)
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> MoveBackButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    const client_t *cl = VM_GetClientForEntRef(entref);

    if (!cl)
        Scr_ObjectError("not a client\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(cl->lastUsercmd.forwardmove < 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_MoveLeftButtonPressed(scr_entref_t entref)
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> MoveLeftButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    const client_t *cl = VM_GetClientForEntRef(entref);

    if (!cl)
        Scr_ObjectError("not a client\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(cl->lastUsercmd.rightmove < 0, SCRIPTINSTANCE_SERVER);
}

void PlayerCmd_MoveRightButtonPressed(scr_entref_t entref)
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Usage: <client> MoveRightButtonPressed()\n", SCRIPTINSTANCE_SERVER);

    const client_t *cl = VM_GetClientForEntRef(entref);

    if (!cl)
        Scr_ObjectError("not a client\n", SCRIPTINSTANCE_SERVER);

    Scr_AddInt(cl->lastUsercmd.rightmove > 0, SCRIPTINSTANCE_SERVER);
}

g_client_script_cmd::g_client_script_cmd()
{
    // Registering methods
    Scr_AddMethod("jumpbuttonpressed", PlayerCmd_JumpButtonPressed, 0);
    Scr_AddMethod("secondaryoffhandbuttonpressed", PlayerCmd_secondaryOffhandButtonPressed, 0);
    Scr_AddMethod("sprintbuttonpressed", PlayerCmd_SprintButtonPressed, 0);
    Scr_AddMethod("moveforwardbuttonpressed", PlayerCmd_MoveForwardButtonPressed, 0);
    Scr_AddMethod("movebackbuttonpressed", PlayerCmd_MoveBackButtonPressed, 0);
    Scr_AddMethod("moveleftbuttonpressed", PlayerCmd_MoveLeftButtonPressed, 0);
    Scr_AddMethod("moverightbuttonpressed", PlayerCmd_MoveRightButtonPressed, 0);
}

g_client_script_cmd::~g_client_script_cmd()
{
}
} // namespace sp
} // namespace t4
