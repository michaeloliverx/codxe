#include "gsc.h"
#include "common.h"

bool DUMP_GSC = false;

namespace t4
{
    namespace sp
    {

        Detour Scr_AddSourceBuffer_Detour;

        char *Scr_AddSourceBuffer_Hook(scriptInstance_t inst, const char *filename, const char *extFilename, const char *codePos, bool archive)
        {
            // filename - the file name without extension, e.g., "example" - paths are unix style e.g. "maps/mp/gametypes/example"
            // extFilename - the file name with extension, e.g., "example.gsc" - paths are unix style e.g. // "maps/mp/gametypes/example.gsc"

            std::string script_path = va("game:/_codxe/raw/%s", extFilename);
            std::replace(script_path.begin(), script_path.end(), '/', '\\'); // Normalize path for Windows/Xbox compatibility

            FILE *file = fopen(script_path.c_str(), "rb");
            if (file)
            {
                DbgPrint("GSCLoader: Found shadowed script file: %s\n", script_path.c_str());
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                rewind(file);

                // Allocate temp buffer from hunk
                auto buffer = (char *)Hunk_AllocateTempMemoryHighInternal(file_size + 1);
                if (buffer)
                {
                    fread(buffer, 1, file_size, file);
                    buffer[file_size] = '\0'; // Null terminate

                    fclose(file);
                    return buffer; // Return overwritten script
                }

                fclose(file);
            }

            auto buffer = Scr_AddSourceBuffer_Detour.GetOriginal<decltype(Scr_AddSourceBuffer)>()(inst, filename, extFilename, codePos, archive);
            if (DUMP_GSC)
            {
                std::string script_dump_path = va("game:/_codxe/dump/%s", extFilename);
                std::replace(script_dump_path.begin(), script_dump_path.end(), '/', '\\'); // Replace forward slashes with backslashes
                DbgPrint("GSCLoader: Dumping script to: %s\n", script_dump_path.c_str());
                filesystem::write_file_to_disk(script_dump_path.c_str(), buffer, std::strlen(buffer));
            }
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
