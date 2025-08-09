#include "common.h"
#include "components/scr_parser.h"

namespace
{
uint32_t ShowKeyboard(const wchar_t *title, const wchar_t *description, const wchar_t *defaultText, std::string &result,
                      size_t maxLength, uint32_t keyboardType)
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
} // namespace

namespace iw3
{
namespace sp
{

Detour CL_GamepadButtonEvent_Detour;

void CL_GamepadButtonEvent_Hook(int localClientNum, int controllerIndex, int key, int down, unsigned int time)
{
    DbgPrint("CL_GamepadButtonEvent localClientNum=%d controllerIndex=%d key=%d down=%d time=%d\n", localClientNum,
             controllerIndex, key, down, time);
    CL_GamepadButtonEvent_Detour.GetOriginal<decltype(CL_GamepadButtonEvent)>()(localClientNum, controllerIndex, key,
                                                                                down, time);

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
        filesystem::write_file_to_disk((std::string(DUMP_DIR) + "\\" + asset_name).c_str(), rawfile->buffer,
                                       rawfile->len);
    }
}

std::vector<Module *> components;

void RegisterComponent(Module *module)
{
    DbgPrint("T4 MP: Component registered: %s\n", module->get_name());
    components.push_back(module);
}

void init()
{
    RegisterComponent(new scr_parser());

    CL_GamepadButtonEvent_Detour = Detour(CL_GamepadButtonEvent, CL_GamepadButtonEvent_Hook);
    CL_GamepadButtonEvent_Detour.Install();

    Load_MapEntsPtr_Detour = Detour(Load_MapEntsPtr, Load_MapEntsPtr_Hook);
    Load_MapEntsPtr_Detour.Install();

    Cmd_AddCommandInternal("dumpraw", Cmd_Dumpraw_f, &Cmd_Dumpraw_f_VAR);
}

void shutdown()
{
    CL_GamepadButtonEvent_Detour.Remove();
    Load_MapEntsPtr_Detour.Remove();

    // TODO: move module loader/unloader logic to a self contained class
    // Clean up in reverse order
    for (auto it = components.rbegin(); it != components.rend(); ++it)
    {
        delete *it;
    }
    components.clear();
}
} // namespace sp
} // namespace iw3
