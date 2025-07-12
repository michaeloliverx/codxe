#include "gsc.h"
#include "common.h"

namespace t4
{
    namespace sp
    {

        Detour Scr_AddSourceBuffer_Detour;

        char *Scr_AddSourceBuffer_Hook(scriptInstance_t inst, const char *filename, const char *extFilename, const char *codePos, bool archive)
        {
            auto callOriginal = [&]()
            {
                return Scr_AddSourceBuffer_Detour.GetOriginal<decltype(Scr_AddSourceBuffer)>()(inst, filename, extFilename, codePos, archive);
            };

            // Check if mod is active
            std::string modBasePath = GetModBasePath();
            if (modBasePath.empty())
                return callOriginal();

            // Build full path to override file
            std::string overridePath = modBasePath + "\\" + extFilename;
            std::replace(overridePath.begin(), overridePath.end(), '/', '\\');

            // Try to load override file
            std::string fileContent = filesystem::read_file_to_string(overridePath);
            if (fileContent.empty())
                return callOriginal();

            // Allocate buffer using game's memory allocator
            char *buffer = (char *)Hunk_AllocateTempMemoryHighInternal(fileContent.size() + 1);
            if (!buffer)
                return callOriginal();

            // Copy content and null terminate
            memcpy(buffer, fileContent.c_str(), fileContent.size());
            buffer[fileContent.size()] = '\0';

            DbgPrint("GSCLoader: Loaded override script: %s\n", overridePath.c_str());
            return buffer;
        }

        client_t *VM_GetClientForEntRef(scr_entref_t entref)
        {
            return &(*reinterpret_cast<client_t **>(0x839EC08C))[entref.entnum];
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

            Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_GOSTAND) != 0, SCRIPTINSTANCE_SERVER);
        }

        void PlayerCmd_secondaryOffhandButtonPressed(scr_entref_t entref)
        {
            if (entref.classnum != 0)
                Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);

            auto ent = &g_entities[entref.entnum];
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

            auto ent = &g_entities[entref.entnum];
            if (!ent->client)
                Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);

            if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
                Scr_Error("Usage: <client> SprintButtonPressed()\n", SCRIPTINSTANCE_SERVER);

            Scr_AddInt(((ent->client->buttonsSinceLastFrame | ent->client->buttons) & KEY_SPRINT) != 0, SCRIPTINSTANCE_SERVER);
        }

        void PlayerCmd_MoveForwardButtonPressed(scr_entref_t object)
        {
            if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
                Scr_Error("Usage: <client> MoveForwardButtonPressed()\n", SCRIPTINSTANCE_SERVER);

            client_t *cl = VM_GetClientForEntRef(object);

            if (!cl)
                Scr_ObjectError("not a client\n", SCRIPTINSTANCE_SERVER);

            Scr_AddInt(cl->lastUsercmd.forwardmove > 0, SCRIPTINSTANCE_SERVER);
        }

        void PlayerCmd_MoveBackButtonPressed(scr_entref_t object)
        {
            if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
                Scr_Error("Usage: <client> MoveBackButtonPressed()\n", SCRIPTINSTANCE_SERVER);

            client_t *cl = VM_GetClientForEntRef(object);

            if (!cl)
                Scr_ObjectError("not a client\n", SCRIPTINSTANCE_SERVER);

            Scr_AddInt(cl->lastUsercmd.forwardmove < 0, SCRIPTINSTANCE_SERVER);
        }

        void PlayerCmd_MoveLeftButtonPressed(scr_entref_t object)
        {
            if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
                Scr_Error("Usage: <client> MoveLeftButtonPressed()\n", SCRIPTINSTANCE_SERVER);

            client_t *cl = VM_GetClientForEntRef(object);

            if (!cl)
                Scr_ObjectError("not a client\n", SCRIPTINSTANCE_SERVER);

            Scr_AddInt(cl->lastUsercmd.rightmove < 0, SCRIPTINSTANCE_SERVER);
        }

        void PlayerCmd_MoveRightButtonPressed(scr_entref_t object)
        {
            if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
                Scr_Error("Usage: <client> MoveRightButtonPressed()\n", SCRIPTINSTANCE_SERVER);

            client_t *cl = VM_GetClientForEntRef(object);

            if (!cl)
                Scr_ObjectError("not a client\n", SCRIPTINSTANCE_SERVER);

            Scr_AddInt(cl->lastUsercmd.rightmove > 0, SCRIPTINSTANCE_SERVER);
        }

        Detour Player_GetMethod_Detour;

        BuiltinMethod Player_GetMethod_Hook(const char **pName)
        {
            if (pName != nullptr)
            {
                if (_stricmp(*pName, "jumpbuttonpressed") == 0)
                    return PlayerCmd_JumpButtonPressed;
                else if (_stricmp(*pName, "secondaryoffhandbuttonpressed") == 0)
                    return PlayerCmd_secondaryOffhandButtonPressed;
                else if (_stricmp(*pName, "sprintbuttonpressed") == 0)
                    return PlayerCmd_SprintButtonPressed;
                else if (_stricmp(*pName, "moveforwardbuttonpressed") == 0)
                    return PlayerCmd_MoveForwardButtonPressed;
                else if (_stricmp(*pName, "movebackbuttonpressed") == 0)
                    return PlayerCmd_MoveBackButtonPressed;
                else if (_stricmp(*pName, "moveleftbuttonpressed") == 0)
                    return PlayerCmd_MoveLeftButtonPressed;
                else if (_stricmp(*pName, "moverightbuttonpressed") == 0)
                    return PlayerCmd_MoveRightButtonPressed;
            }
            return Player_GetMethod_Detour.GetOriginal<decltype(Player_GetMethod)>()(pName);
        }

        GSC::GSC()
        {
            DbgPrint("GSC initialized\n");

            Scr_AddSourceBuffer_Detour = Detour(Scr_AddSourceBuffer, Scr_AddSourceBuffer_Hook);
            Scr_AddSourceBuffer_Detour.Install();

            Player_GetMethod_Detour = Detour(Player_GetMethod, Player_GetMethod_Hook);
            Player_GetMethod_Detour.Install();
        }

        GSC::~GSC()
        {
            DbgPrint("GSC shutdown\n");

            Scr_AddSourceBuffer_Detour.Remove();
        }
    }
}
