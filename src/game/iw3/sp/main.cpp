#include "common.h"

namespace
{
    uint32_t ShowKeyboard(const wchar_t *title, const wchar_t *description, const wchar_t *defaultText, std::string &result, size_t maxLength, uint32_t keyboardType)
    {
        size_t realMaxLength = maxLength + 1;
        XOVERLAPPED overlapped = {};

        std::vector<wchar_t> wideBuffer(realMaxLength);
        std::vector<char> buffer(realMaxLength);

        XShowKeyboardUI(0, keyboardType, defaultText, title, description, wideBuffer.data(), realMaxLength, &overlapped);

        while (!XHasOverlappedIoCompleted(&overlapped))
            Sleep(100);

        if (XGetOverlappedResult(&overlapped, nullptr, TRUE) == ERROR_SUCCESS)
        {
            wcstombs_s(nullptr, buffer.data(), realMaxLength, wideBuffer.data(), realMaxLength * sizeof(wchar_t));
            result = buffer.data();
            return ERROR_SUCCESS;
        }

        return ERROR_CANCELLED;
    }
}

namespace iw3
{

    namespace sp
    {
        // Functions
        Cbuf_AddText_t Cbuf_AddText = reinterpret_cast<Cbuf_AddText_t>(0x821DE9D8);

        CL_ConsolePrint_t CL_ConsolePrint = reinterpret_cast<CL_ConsolePrint_t>(0x822D7040);
        CL_GamepadButtonEvent_t CL_GamepadButtonEvent = reinterpret_cast<CL_GamepadButtonEvent_t>(0x822CE630);

        Cmd_AddCommandInternal_t Cmd_AddCommandInternal = reinterpret_cast<Cmd_AddCommandInternal_t>(0x821DFAD0);

        Com_PrintError_t Com_PrintError = reinterpret_cast<Com_PrintError_t>(0x821DAC90);
        Com_Printf_t Com_Printf = reinterpret_cast<Com_Printf_t>(0x821DC0A0);
        Com_PrintWarning_t Com_PrintWarning = reinterpret_cast<Com_PrintWarning_t>(0x821DA798);

        DB_EnumXAssets_FastFile_t DB_EnumXAssets_FastFile = reinterpret_cast<DB_EnumXAssets_FastFile_t>(0x822AEF88);

        Load_MapEntsPtr_t Load_MapEntsPtr = reinterpret_cast<Load_MapEntsPtr_t>(0x822B9788);

        Scr_AddSourceBuffer_t Scr_AddSourceBuffer = reinterpret_cast<Scr_AddSourceBuffer_t>(0x821C5A18);
        Scr_ReadFile_FastFile_t Scr_ReadFile_FastFile = reinterpret_cast<Scr_ReadFile_FastFile_t>(0x821C5978);

        // Variables
        auto cmd_functions = reinterpret_cast<cmd_function_s *>(0x82DDEFCC);

        Detour CL_ConsolePrint_Detour;

        void CL_ConsolePrint_Hook(int localClientNum, int channel, const char *txt, int duration, int pixelWidth, int flags)
        {
            CL_ConsolePrint_Detour.GetOriginal<decltype(CL_ConsolePrint)>()(localClientNum, channel, txt, duration, pixelWidth, flags);
            DbgPrint("CL_ConsolePrint txt=%s \n", txt);
        }

        Detour CL_GamepadButtonEvent_Detour;

        void CL_GamepadButtonEvent_Hook(int localClientNum, int controllerIndex, int key, int down, unsigned int time)
        {
            DbgPrint("CL_GamepadButtonEvent localClientNum=%d controllerIndex=%d key=%d down=%d time=%d\n", localClientNum, controllerIndex, key, down, time);
            CL_GamepadButtonEvent_Detour.GetOriginal<decltype(CL_GamepadButtonEvent)>()(localClientNum, controllerIndex, key, down, time);

            if (key == K_BUTTON_RSTICK && down)
            {
                std::string value;
                auto result = ShowKeyboard(L"Title", L"Description", L"Default Text", value, 100, 0);
                if (result == ERROR_SUCCESS)
                {
                    DbgPrint("ShowKeyboard result: %s\n", value.c_str());
                    Cbuf_AddText(0, value.c_str());
                }
                else
                {
                    DbgPrint("ShowKeyboard cancelled.\n");
                }
            }
        }

        Detour Load_MapEntsPtr_Detour;

        void Load_MapEntsPtr_Hook()
        {
            // TODO: don't write null byte to file
            // and add null byte to entityString when reading from file

            DbgPrint("Load_MapEntsPtr_Hook\n");

            // TODO: write comment what this is ***
            // Get pointer to pointer stored at 0x82475914
            MapEnts **varMapEntsPtr = *(MapEnts ***)0x825875D8;

            Load_MapEntsPtr_Detour.GetOriginal<decltype(&Load_MapEntsPtr_Hook)>()();

            // Validate pointer before dereferencing
            if (varMapEntsPtr && *varMapEntsPtr)
            {
                MapEnts *mapEnts = *varMapEntsPtr;

                // Write stock map ents to disk
                std::string file_path = "game:\\dump\\";
                file_path += mapEnts->name;
                file_path += ".ents";                                        //  iw4x naming convention
                std::replace(file_path.begin(), file_path.end(), '/', '\\'); // Replace forward slashes with backslashes
                filesystem::write_file_to_disk(file_path.c_str(), mapEnts->entityString, mapEnts->numEntityChars);

                // Load map ents from file
                // Path to check for existing entity file
                std::string raw_file_path = "game:\\raw\\";
                raw_file_path += mapEnts->name;
                raw_file_path += ".ents";                                            // IW4x naming convention
                std::replace(raw_file_path.begin(), raw_file_path.end(), '/', '\\'); // Replace forward slashes with backslashes

                // If the file exists, replace entityString
                if (filesystem::file_exists(raw_file_path))
                {
                    DbgPrint("Found entity file: %s\n", raw_file_path.c_str());
                    std::string new_entity_string = filesystem::read_file_to_string(raw_file_path);
                    if (!new_entity_string.empty())
                    {
                        // Allocate new memory and copy the data
                        size_t new_size = new_entity_string.size() + 1; // Include null terminator
                        char *new_memory = static_cast<char *>(malloc(new_size));

                        if (new_memory)
                        {
                            memcpy(new_memory, new_entity_string.c_str(), new_size); // Copy with null terminator
                            mapEnts->entityString = new_memory;
                            DbgPrint("Replaced entityString from file: %s\n", raw_file_path.c_str());
                        }
                        else
                        {
                            DbgPrint("Failed to allocate memory for entityString replacement.\n");
                        }
                    }
                }
            }
            else
            {
                DbgPrint("Hooked Load_MapEntsPtr: varMapEntsPtr is NULL or invalid.\n");
            }
        }

        Detour Scr_ReadFile_FastFile_Detour;

        char *Scr_ReadFile_FastFile_Hook(const char *filename, const char *extFilename, const char *codePos, bool archive)
        {
            DbgPrint("Scr_ReadFile_FastFile_Hook extFilename=%s \n", extFilename);

            std::string raw_file_path = "game:\\raw\\";
            raw_file_path += extFilename;
            std::replace(raw_file_path.begin(), raw_file_path.end(), '/', '\\'); // Replace forward slashes with backslashes
            if (filesystem::file_exists(raw_file_path))
            {
                DbgPrint("Found raw file: %s\n", raw_file_path.c_str());
                // return ReadFileContents(raw_file_path);
                std::string new_contents = filesystem::read_file_to_string(raw_file_path);
                if (!new_contents.empty())
                {

                    // Allocate new memory and copy the data
                    size_t new_size = new_contents.size() + 1; // Include null terminator
                    char *new_memory = static_cast<char *>(malloc(new_size));

                    if (new_memory)
                    {
                        memcpy(new_memory, new_contents.c_str(), new_size); // Copy with null terminator

                        DbgPrint("Replaced contents from file: %s\n", raw_file_path.c_str());
                        return new_memory;
                    }
                    else
                    {
                        DbgPrint("Failed to allocate memory for contents replacement.\n");
                    }
                }
            }

            return Scr_ReadFile_FastFile_Detour.GetOriginal<decltype(&Scr_ReadFile_FastFile_Hook)>()(filename, extFilename, codePos, archive);
        }

        const unsigned int MAX_RAWFILES = 2048;
        struct RawFileList
        {
            unsigned int count;
            RawFile *files[MAX_RAWFILES];
        };

        void R_AddRawFileToList(void *asset, void *inData)
        {
            RawFileList *rawFileList = reinterpret_cast<RawFileList *>(inData);
            RawFile *rawFile = reinterpret_cast<RawFile *>(asset);

            if (!rawFile)
            {
                Com_PrintError(CON_CHANNEL_ERROR, "R_AddRawFileToList: Null RawFile!\n");
                return;
            }

            if (rawFileList->count >= MAX_RAWFILES)
            {
                Com_PrintError(CON_CHANNEL_ERROR, "R_AddRawFileToList: RawFileList is full!\n");
                return;
            }

            rawFileList->files[rawFileList->count++] = rawFile;
        }

        void R_GetRawFileList(RawFileList *rawFileList)
        {
            rawFileList->count = 0;
            DB_EnumXAssets_FastFile(ASSET_TYPE_RAWFILE, R_AddRawFileToList, rawFileList, true);
        }

        void Cmd_rawfilesdump()
        {
            RawFileList rawFileList;
            R_GetRawFileList(&rawFileList);

            Com_Printf(CON_CHANNEL_CONSOLEONLY, "Dumping %d raw files to `raw\\` %d\n", rawFileList.count);

            for (unsigned int i = 0; i < rawFileList.count; i++)
            {
                auto rawfile = rawFileList.files[i];
                std::string asset_name = rawfile->name;
                std::replace(asset_name.begin(), asset_name.end(), '/', '\\'); // Replace forward slashes with backslashes
                filesystem::write_file_to_disk(("game:\\dump\\" + asset_name).c_str(), rawfile->buffer, rawfile->len);
            }
        }

        void init()
        {
            DbgPrint("Initializing SP\n");

            CL_ConsolePrint_Detour = Detour(CL_ConsolePrint, CL_ConsolePrint_Hook);
            CL_ConsolePrint_Detour.Install();

            CL_GamepadButtonEvent_Detour = Detour(CL_GamepadButtonEvent, CL_GamepadButtonEvent_Hook);
            CL_GamepadButtonEvent_Detour.Install();

            Load_MapEntsPtr_Detour = Detour(Load_MapEntsPtr, Load_MapEntsPtr_Hook);
            Load_MapEntsPtr_Detour.Install();

            Scr_ReadFile_FastFile_Detour = Detour(Scr_ReadFile_FastFile, Scr_ReadFile_FastFile_Hook);
            Scr_ReadFile_FastFile_Detour.Install();

            cmd_function_s *rawfilesdump_VAR = new cmd_function_s;
            Cmd_AddCommandInternal("rawfilesdump", Cmd_rawfilesdump, rawfilesdump_VAR);
        }
    }
}
