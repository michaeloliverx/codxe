#include "scr_parser.h"
#include "common.h"

namespace t6
{
    namespace mp
    {

        // Detour Load_ScriptParseTreeAsset_Detour;
        // void Load_ScriptParseTreeAsset_Hook(ScriptParseTree **a1)
        // {
        //     ScriptParseTree *scriptParseTree = *a1;
        //     DbgPrint("GSCLoader: Loading script %s\n", scriptParseTree->name);

        //     // DbgPrint("GSCLoader: Dumping script %s\n", scriptParseTree->name);
        //     // std::string dumpPath = std::string(DUMP_DIR) + "\\" + scriptParseTree->name;
        //     // std::replace(dumpPath.begin(), dumpPath.end(), '/', '\\');
        //     // filesystem::write_file_to_disk(dumpPath.c_str(), scriptParseTree->buffer, scriptParseTree->len);
        //     // DbgPrint("GSCLoader: Dumped script to %s\n", dumpPath.c_str());

        //     Config config;
        //     LoadConfigFromFile(CONFIG_PATH, config);
        //     std::string modBasePath = config.GetModBasePath();
        //     if (!modBasePath.empty())
        //     {
        //         std::string overridePath = modBasePath + "\\" + scriptParseTree->name + "bin";
        //         DbgPrint("GSCLoader: Checking for override script at %s\n", overridePath.c_str());
        //     }

        //     Load_ScriptParseTreeAsset_Detour.GetOriginal<decltype(Load_ScriptParseTreeAsset)>()(a1);
        // }

        Detour Load_ScriptParseTreeAsset_Detour;

        void Load_ScriptParseTreeAsset_Hook(ScriptParseTree **a1)
        {
            ScriptParseTree *scriptParseTree = *a1;
            DbgPrint("GSCLoader: Loading script %s\n", scriptParseTree->name);

            Config config;
            LoadConfigFromFile(CONFIG_PATH, config);
            std::string modBasePath = config.GetModBasePath();
            if (!modBasePath.empty())
            {
                std::string overridePath = modBasePath + "\\" + scriptParseTree->name + "bin";
                std::replace(overridePath.begin(), overridePath.end(), '/', '\\');
                DbgPrint("GSCLoader: Checking for override script at %s\n", overridePath.c_str());

                HANDLE file = CreateFileA(
                    overridePath.c_str(),
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);

                if (file != INVALID_HANDLE_VALUE)
                {
                    DWORD fileSize = GetFileSize(file, NULL);
                    if (fileSize != INVALID_FILE_SIZE && fileSize > 0)
                    {
                        char *buffer = new char[fileSize];
                        DWORD bytesRead = 0;
                        if (ReadFile(file, buffer, fileSize, &bytesRead, NULL) && bytesRead == fileSize)
                        {
                            scriptParseTree->buffer = buffer;
                            scriptParseTree->len = fileSize;
                            DbgPrint("GSCLoader: Overridden script loaded (%d bytes)\n", fileSize);
                        }
                        else
                        {
                            delete[] buffer;
                            DbgPrint("GSCLoader: Failed to read override script\n");
                        }
                    }
                    CloseHandle(file);
                }
            }

            Load_ScriptParseTreeAsset_Detour.GetOriginal<decltype(Load_ScriptParseTreeAsset)>()(a1);
        }

        scr_parser::scr_parser()
        {
            Load_ScriptParseTreeAsset_Detour = Detour(Load_ScriptParseTreeAsset, Load_ScriptParseTreeAsset_Hook);
            Load_ScriptParseTreeAsset_Detour.Install();
        }

        scr_parser::~scr_parser()
        {
            Load_ScriptParseTreeAsset_Detour.Remove();
        }
    }
}
