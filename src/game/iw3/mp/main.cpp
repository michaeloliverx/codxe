#include "pch.h"
#include "components/cg.h"
#include "components/cj_tas.h"
#include "components/clipmap.h"
#include "components/cmds.h"
#include "components/events.h"
#include "components/g_scr_main.h"
#include "components/gsc_client_fields.h"
#include "components/gsc_functions.h"
#include "components/gsc_hud_elem.h"
#include "components/gsc_methods.h"
#include "components/image_loader.h"
#include "components/mpsp.h"
#include "components/pm.h"
#include "components/scr_parser.h"
#include "components/sv_bots.h"
#include "common/config.h"
#include "main.h"

// Structure to hold data for the active keyboard request
struct KeyboardRequest
{
    wchar_t *resultText;
    XOVERLAPPED overlapped;
    std::function<void(bool, const wchar_t *)> callback;
    DWORD maxLength;
    bool isActive;

    KeyboardRequest() : resultText(nullptr), callback(nullptr), maxLength(0), isActive(false)
    {
        ZeroMemory(&overlapped, sizeof(XOVERLAPPED));
    }

    ~KeyboardRequest()
    {
        if (resultText)
        {
            delete[] resultText;
            resultText = nullptr;
        }
    }
};

// Global singleton for the keyboard request
static KeyboardRequest g_keyboardRequest;

// Function to show keyboard UI without blocking
bool ShowKeyboardAsync(DWORD dwUserIndex, const wchar_t *defaultText, const wchar_t *titleText,
                       const wchar_t *descriptionText, DWORD maxTextLength, DWORD keyboardType,
                       std::function<void(bool, const wchar_t *)> callback)
{
    // If there's already an active keyboard request, don't allow another one
    if (g_keyboardRequest.isActive)
    {
        DbgPrint("Keyboard UI is already active, ignoring request\n");
        return false;
    }

    // Clean up any previous request
    if (g_keyboardRequest.resultText)
    {
        delete[] g_keyboardRequest.resultText;
        g_keyboardRequest.resultText = nullptr;
    }

    // Allocate new result text buffer (add 1 for null terminator)
    DWORD bufferSize = maxTextLength + 1;
    g_keyboardRequest.resultText = new wchar_t[bufferSize];
    ZeroMemory(g_keyboardRequest.resultText, bufferSize * sizeof(wchar_t));
    g_keyboardRequest.maxLength = bufferSize;
    g_keyboardRequest.callback = callback;

    // Zero out the overlapped structure
    ZeroMemory(&g_keyboardRequest.overlapped, sizeof(XOVERLAPPED));

    // Set up the keyboard UI with overlapped to make it non-blocking
    HRESULT hr = XShowKeyboardUI(dwUserIndex, keyboardType, defaultText, titleText, descriptionText,
                                 g_keyboardRequest.resultText, bufferSize, &g_keyboardRequest.overlapped);

    if (FAILED(hr))
    {
        // Handle error
        DbgPrint("Failed to show keyboard UI: 0x%08X\n", hr);
        delete[] g_keyboardRequest.resultText;
        g_keyboardRequest.resultText = nullptr;
        return false;
    }

    // Mark as active
    g_keyboardRequest.isActive = true;
    DbgPrint("Keyboard UI requested, continuing with game loop...\n");
    return true;
}

// Function to call in your game loop to check for keyboard completion
void CheckKeyboardCompletion()
{
    // If no active keyboard request, nothing to do
    if (!g_keyboardRequest.isActive)
    {
        return;
    }

    // Check if the operation has completed
    if (XHasOverlappedIoCompleted(&g_keyboardRequest.overlapped))
    {
        DWORD result;
        XGetOverlappedResult(&g_keyboardRequest.overlapped, &result, TRUE);

        DbgPrint("Keyboard operation completed with result: 0x%08X\n", result);

        // Operation completed
        bool success = (result == ERROR_SUCCESS);

        // Call the user callback with the result
        if (g_keyboardRequest.callback)
        {
            if (success)
            {
                DbgPrint("Keyboard text entry successful\n");
                g_keyboardRequest.callback(true, g_keyboardRequest.resultText);
            }
            else
            {
                DbgPrint("Keyboard operation failed or canceled\n");
                g_keyboardRequest.callback(false, nullptr);
            }
        }

        // Clean up
        delete[] g_keyboardRequest.resultText;
        g_keyboardRequest.resultText = nullptr;
        g_keyboardRequest.isActive = false;
    }
    // If not completed, we'll check again next frame
}

namespace iw3
{
namespace mp
{
void Cmd_cmdinput_f()
{
    bool success = ShowKeyboardAsync(0,                // First controller
                                     L"",              // Default text
                                     L"Enter command", // Title
                                     L"",              // Description
                                     256,              // Max length
                                     VKBD_DEFAULT,     // Keyboard type
                                     [](bool success, const wchar_t *text)
                                     {
                                         if (success && text)
                                         {
                                             // Get the required buffer size
                                             size_t wideLength = wcslen(text);
                                             size_t mbBufferSize =
                                                 wideLength * 4 + 1; // 4 bytes per character worst case

                                             // Create buffer and convert
                                             std::vector<char> mbBuffer(mbBufferSize);
                                             wcstombs_s(nullptr, mbBuffer.data(), mbBufferSize, text, wideLength);

                                             // Create string from the buffer
                                             std::string result = mbBuffer.data();

                                             Cbuf_AddText(0, result.c_str());
                                         }
                                         else
                                         {
                                             DbgPrint("Keyboard operation failed or was canceled\n");
                                         }
                                     });

    if (!success)
    {
        DbgPrint("Failed to open keyboard UI\n");
    }
}

Detour CL_GamepadButtonEvent_Detour;

void CL_GamepadButtonEvent_Hook(int localClientNum, int controllerIndex, int key, int down, unsigned int time)
{
    CL_GamepadButtonEvent_Detour.GetOriginal<decltype(CL_GamepadButtonEvent)>()(localClientNum, controllerIndex, key,
                                                                                down, time);

    // Check if the client is disconnected (main menu)
    if (clientUIActives[localClientNum].connectionState == CA_DISCONNECTED)
    {
        if (key == K_BUTTON_RSTICK && down)
        {
            Cmd_cmdinput_f();
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
    MapEnts **varMapEntsPtr = *(MapEnts ***)0x82475914;

    Load_MapEntsPtr_Detour.GetOriginal<decltype(&Load_MapEntsPtr_Hook)>()();

    // Validate pointer before dereferencing
    if (varMapEntsPtr && *varMapEntsPtr)
    {
        MapEnts *mapEnts = *varMapEntsPtr;

        // Write stock map ents to disk
        std::string file_path = DUMP_DIR;
        file_path += std::string("\\") + mapEnts->name;
        file_path += ".ents";                                        //  iw4x naming convention
        std::replace(file_path.begin(), file_path.end(), '/', '\\'); // Replace forward slashes with backslashes
        filesystem::write_file_to_disk(file_path.c_str(), mapEnts->entityString, mapEnts->numEntityChars);

        // Use new ConfigModule API
        // Load map ents from file
        // Path to check for existing entity file
        std::string raw_file_path = Config::GetModBasePath();

        raw_file_path += std::string("\\") + mapEnts->name;
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

                    // Update the entityString pointer to point to the new memory
                    mapEnts->entityString = new_memory;

                    // // Update numEntityChars
                    // mapEnts->numEntityChars = static_cast<int>(new_entity_string.size());	// unnecessary

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

Detour UI_Refresh_Detour;

void UI_Refresh_Hook(int localClientNum)
{
    UI_Refresh_Detour.GetOriginal<decltype(UI_Refresh)>()(localClientNum);
    CheckKeyboardCompletion();
}

/**
 * Patch out the signature checks used during fastfile authentication.
 * Signature data must still be present in the fastfile structure, but the values themselves may be zeroed.
 */
void DisableFastfileAuth()
{
    // DBX_AuthLoad_ValidateHash
    *(volatile uint32_t *)0x822B2994 = 0x60000000;
    *(volatile uint32_t *)0x822B2A34 = 0x60000000;
    *(volatile uint32_t *)0x822B2D2C = 0x60000000;

    // DBX_AuthLoad_ValidateSignature
    *(volatile uint32_t *)0x822B2D44 = 0x60000000;
}

IW3_MP_Plugin::IW3_MP_Plugin()
{
    DisableFastfileAuth();

    // Special modules need to be registered first
    RegisterModule(new Config());
    RegisterModule(new Events());

    RegisterModule(new cg());
    RegisterModule(new cj_tas());
    RegisterModule(new clipmap());
    RegisterModule(new cmds());
    RegisterModule(new g_scr_main());
    RegisterModule(new gsc_client_fields());
    RegisterModule(new gsc_functions());
    RegisterModule(new gsc_hud_elem());
    RegisterModule(new gsc_methods());
    RegisterModule(new image_loader());
    RegisterModule(new pm());
    RegisterModule(new mpsp());
    RegisterModule(new scr_parser());
    RegisterModule(new sv_bots());

    UI_Refresh_Detour = Detour(UI_Refresh, UI_Refresh_Hook);
    UI_Refresh_Detour.Install();

    CL_GamepadButtonEvent_Detour = Detour(CL_GamepadButtonEvent, CL_GamepadButtonEvent_Hook);
    CL_GamepadButtonEvent_Detour.Install();

    Load_MapEntsPtr_Detour = Detour(Load_MapEntsPtr, Load_MapEntsPtr_Hook);
    Load_MapEntsPtr_Detour.Install();

    cmd_function_s *cmdinput_VAR = new cmd_function_s;
    Cmd_AddCommandInternal("cmdinput", Cmd_cmdinput_f, cmdinput_VAR);

    Events::OnCG_DrawActive([]() { CheckKeyboardCompletion(); });
}

IW3_MP_Plugin::~IW3_MP_Plugin()
{
    UI_Refresh_Detour.Remove();
    CL_GamepadButtonEvent_Detour.Remove();
    Load_MapEntsPtr_Detour.Remove();
}

} // namespace mp
} // namespace iw3
