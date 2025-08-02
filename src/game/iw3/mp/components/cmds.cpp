#include "cmds.h"
#include "common.h"

namespace iw3
{
    namespace mp
    {
        static cmd_function_s Cmd_Dumpraw_f_VAR;

        void Cmd_Dumpraw_f()
        {
            XAssetHeader files[2048];
            auto count = DB_GetAllXAssetOfType_FastFile(ASSET_TYPE_RAWFILE, files, 2048);

            for (int i = 0; i < count; i++)
            {
                auto rawfile = files[i].rawfile;
                std::string asset_name = rawfile->name;
                std::replace(asset_name.begin(), asset_name.end(), '/', '\\'); // Replace forward slashes with backslashes
                filesystem::write_file_to_disk((std::string(DUMP_DIR) + "\\" + asset_name).c_str(), rawfile->buffer, rawfile->len);
            }
        }

        // Client commands

        void Cmd_UFO_f(gentity_s *ent)
        {
            if (!CheatsOk(ent))
                return;

            gclient_s *client = ent->client;

            bool enableUFO = !client->ufo;
            client->ufo = enableUFO;

            const char *message = enableUFO ? "GAME_UFOON" : "GAME_UFOOFF";

            // Format the command string (note: 101 is ASCII for 'e')
            const char *commandString = va("%c \"%s\"", 101, message);

            int entityIndex = ent - g_entities;

            if (entityIndex == -1)
                SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, "%s", commandString);
            else
                SV_GameSendServerCommand(entityIndex, SV_CMD_CAN_IGNORE, commandString);
        }

        void Cmd_Noclip_f(gentity_s *ent)
        {
            if (!CheatsOk(ent))
                return;

            gclient_s *client = ent->client;

            bool enableNoclip = !client->noclip;
            client->noclip = enableNoclip;

            const char *message = enableNoclip ? "GAME_NOCLIPON" : "GAME_NOCLIPOFF";

            // Format the command string (note: 101 is ASCII for 'e')
            const char *commandString = va("%c \"%s\"", 101, message);

            int entityIndex = ent - g_entities;

            if (entityIndex == -1)
                SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, "%s", commandString);
            else
                SV_GameSendServerCommand(entityIndex, SV_CMD_CAN_IGNORE, commandString);
        }

        void Cmd_God_f(gentity_s *ent)
        {
            if (!CheatsOk(ent))
                return;

            // Toggle god mode flag (bit 0)
            ent->flags ^= 1;

            bool godModeEnabled = (ent->flags & 1) != 0;
            const char *message = godModeEnabled ? "GAME_GODMODE_ON" : "GAME_GODMODE_OFF";

            // Format command string (101 is ASCII for 'e')
            const char *commandString = va("%c \"%s\"", 101, message);

            int entityIndex = ent - g_entities;

            if (entityIndex == -1)
                SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, "%s", commandString);
            else
                SV_GameSendServerCommand(entityIndex, SV_CMD_CAN_IGNORE, commandString);
        }

        Detour ClientCommand_Detour;

        void ClientCommand_Hook(int clientNum)
        {
            gentity_s *ent = &g_entities[clientNum];

            char cmd[1032];
            SV_Cmd_ArgvBuffer(0, cmd, 1024);

            if (I_strnicmp(cmd, "noclip", 6) == 0)
                Cmd_Noclip_f(ent);
            else if (I_strnicmp(cmd, "ufo", 3) == 0)
                Cmd_UFO_f(ent);
            else if (I_strnicmp(cmd, "god", 3) == 0)
                Cmd_God_f(ent);
            else
                ClientCommand_Detour.GetOriginal<decltype(ClientCommand)>()(clientNum);
        }

        Detour Cmd_ExecFromFastFile_Detour;

        bool Cmd_ExecFromFastFile_Hook(int localClientNum, int controllerIndex, const char *filename)
        {
            auto callOriginal = [&]()
            {
                return Cmd_ExecFromFastFile_Detour.GetOriginal<decltype(Cmd_ExecFromFastFile)>()(localClientNum, controllerIndex, filename);
            };

            Config config;
            LoadConfigFromFile(CONFIG_PATH, config);

            // Check if mod is active
            std::string modBasePath = config.GetModBasePath();
            if (modBasePath.empty())
                return callOriginal();

            std::string contents = filesystem::read_file_to_string(modBasePath + "\\" + filename);
            if (contents.empty())
                return callOriginal();

            Com_Printf(CON_CHANNEL_SYSTEM, "execing %s from raw:\\\n", filename);
            Cbuf_ExecuteBuffer(localClientNum, controllerIndex, contents.c_str());
            return true;
        }

        cmds::cmds()
        {
            ClientCommand_Detour = Detour(ClientCommand, ClientCommand_Hook);
            ClientCommand_Detour.Install();

            Cmd_ExecFromFastFile_Detour = Detour(Cmd_ExecFromFastFile, Cmd_ExecFromFastFile_Hook);
            Cmd_ExecFromFastFile_Detour.Install();

            Cmd_AddCommandInternal("dumpraw", Cmd_Dumpraw_f, &Cmd_Dumpraw_f_VAR);
        }

        cmds::~cmds()
        {
            ClientCommand_Detour.Remove();

            Cmd_ExecFromFastFile_Detour.Remove();
        }
    }
}
