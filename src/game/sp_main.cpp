#include <algorithm>
#include <vector>

#include "sp_main.h"
#include "sp_structs.h"

#include "../detour.h"
#include "../filesystem.h"
#include "../xboxkrnl.h"

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

namespace sp
{
    Cbuf_AddText_t Cbuf_AddText = reinterpret_cast<Cbuf_AddText_t>(0x821DE9D8);

    CL_ConsolePrint_t CL_ConsolePrint = reinterpret_cast<CL_ConsolePrint_t>(0x822D7040);
    CL_GamepadButtonEvent_t CL_GamepadButtonEvent = reinterpret_cast<CL_GamepadButtonEvent_t>(0x822CE630);

    Cmd_AddCommandInternal_t Cmd_AddCommandInternal = reinterpret_cast<Cmd_AddCommandInternal_t>(0x821DFAD0);

    Com_Printf_t Com_Printf = reinterpret_cast<Com_Printf_t>(0x821DC0A0);

    Scr_AddSourceBuffer_t Scr_AddSourceBuffer = reinterpret_cast<Scr_AddSourceBuffer_t>(0x821C5A18);
    Scr_ReadFile_FastFile_t Scr_ReadFile_FastFile = reinterpret_cast<Scr_ReadFile_FastFile_t>(0x821C5978);

    auto cmd_functions = reinterpret_cast<cmd_function_s *>(0x82DDEFCC);

    Detour CL_ConsolePrint_Detour;

    void CL_ConsolePrint_Hook(int localClientNum, int channel, const char *txt, int duration, int pixelWidth, int flags)
    {
        CL_ConsolePrint_Detour.GetOriginal<decltype(CL_ConsolePrint)>()(localClientNum, channel, txt, duration, pixelWidth, flags);
        xbox::DbgPrint("CL_ConsolePrint txt=%s \n", txt);
    }

    Detour CL_GamepadButtonEvent_Detour;

    void CL_GamepadButtonEvent_Hook(int localClientNum, int controllerIndex, int key, int down, unsigned int time)
    {
        xbox::DbgPrint("CL_GamepadButtonEvent localClientNum=%d controllerIndex=%d key=%d down=%d time=%d\n", localClientNum, controllerIndex, key, down, time);
        CL_GamepadButtonEvent_Detour.GetOriginal<decltype(CL_GamepadButtonEvent)>()(localClientNum, controllerIndex, key, down, time);

        if (key == K_BUTTON_RSTICK && down)
        {
            std::string value;
            auto result = ShowKeyboard(L"Title", L"Description", L"Default Text", value, 100, 0);
            if (result == ERROR_SUCCESS)
            {
                xbox::DbgPrint("ShowKeyboard result: %s\n", value.c_str());
                Cbuf_AddText(0, value.c_str());
            }
            else
            {
                xbox::DbgPrint("ShowKeyboard cancelled.\n");
            }
        }
    }

    Detour Scr_ReadFile_FastFile_Detour;

    char *Scr_ReadFile_FastFile_Hook(const char *filename, const char *extFilename, const char *codePos, bool archive)
    {
        xbox::DbgPrint("Scr_ReadFile_FastFile_Hook extFilename=%s \n", extFilename);

        auto contents = Scr_ReadFile_FastFile_Detour.GetOriginal<decltype(&Scr_ReadFile_FastFile_Hook)>()(filename, extFilename, codePos, archive);

        // Dump the file to disk if it exists
        // It might not exist if it's not a stock file
        if (contents != nullptr)
        {
            // Dump the file to disk
            std::string file_path = "game:\\dump\\";
            file_path += extFilename;
            std::replace(file_path.begin(), file_path.end(), '/', '\\'); // Replace forward slashes with backslashes
            filesystem::write_file_to_disk(file_path.c_str(), contents, strlen(contents));
        }

        std::string raw_file_path = "game:\\raw\\";
        raw_file_path += extFilename;
        std::replace(raw_file_path.begin(), raw_file_path.end(), '/', '\\'); // Replace forward slashes with backslashes
        if (filesystem::file_exists(raw_file_path))
        {
            xbox::DbgPrint("Found raw file: %s\n", raw_file_path.c_str());
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

                    xbox::DbgPrint("Replaced contents from file: %s\n", raw_file_path.c_str());
                    return new_memory;
                }
                else
                {
                    xbox::DbgPrint("Failed to allocate memory for contents replacement.\n");
                }
            }
        }

        return contents;
    }

    void Cmd_test_f()
    {
        xbox::DbgPrint("test command\n");
    }

    void init()
    {
        xbox::DbgPrint("Initializing SP\n");

        CL_ConsolePrint_Detour = Detour(CL_ConsolePrint, CL_ConsolePrint_Hook);
        CL_ConsolePrint_Detour.Install();

        CL_GamepadButtonEvent_Detour = Detour(CL_GamepadButtonEvent, CL_GamepadButtonEvent_Hook);
        CL_GamepadButtonEvent_Detour.Install();

        Scr_ReadFile_FastFile_Detour = Detour(Scr_ReadFile_FastFile, Scr_ReadFile_FastFile_Hook);
        Scr_ReadFile_FastFile_Detour.Install();

        cmd_function_s *test_cmd = new cmd_function_s;

        Cmd_AddCommandInternal("test", Cmd_test_f, test_cmd);
    }
}
