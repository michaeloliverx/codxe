#include <xtl.h>
#include <cstdint>
#include <string>
#include <direct.h>
#include <stdio.h>
#include <fstream>
#include <algorithm>

#include "detour.h"

// Declarations for Xbox 360 kernel functions not exposed in the official SDK.
extern "C"
{
	void DbgPrint(const char *format, ...);

	uint32_t ExCreateThread(
		HANDLE *pHandle,
		uint32_t stackSize,
		uint32_t *pThreadId,
		void *pApiThreadStartup,
		PTHREAD_START_ROUTINE pStartAddress,
		void *pParameter,
		uint32_t creationFlags);

	uint32_t XamGetCurrentTitleId();
}

void *ResolveFunction(const std::string &moduleName, uint32_t ordinal)
{
	HMODULE moduleHandle = GetModuleHandle(moduleName.c_str());
	if (moduleHandle == nullptr)
		return nullptr;

	return GetProcAddress(moduleHandle, reinterpret_cast<const char *>(ordinal));
}

typedef void (*XNOTIFYQUEUEUI)(uint32_t type, uint32_t userIndex, uint64_t areas, const wchar_t *displayText, void *pContextData);
XNOTIFYQUEUEUI XNotifyQueueUI = static_cast<XNOTIFYQUEUEUI>(ResolveFunction("xam.xex", 656));

/**
 * Recursively creates nested directories.
 *
 * @param path The full directory path to create.
 */
void create_nested_dirs(const char *path)
{
	char temp_path[256];
	strncpy(temp_path, path, sizeof(temp_path));

	char *p = temp_path;
	while (*p)
	{
		if (*p == '\\' || *p == '/')
		{
			*p = '\0';

			// Create the directory if it doesn't exist
			_mkdir(temp_path);

			*p = '\\';
		}
		p++;
	}

	_mkdir(temp_path);
}

/**
 * Writes data to a file on disk.
 *
 * @param file_path The full path to the file to write.
 * @param data The data to write.
 * @param data_size The size of the data in bytes.
 * @return 1 if successful, 0 if failed.
 */
int write_file_to_disk(const char *file_path, const char *data, size_t data_size)
{
	if (!file_path || !data || data_size == 0)
	{
		DbgPrint("write_file_to_disk: Invalid parameters!\n");
		return 0;
	}

	// Ensure the directory exists
	char dir_path[256];
	strncpy(dir_path, file_path, sizeof(dir_path));
	char *last_slash = strrchr(dir_path, '\\');
	if (last_slash)
	{
		*last_slash = '\0';
		create_nested_dirs(dir_path);
	}

	// Write data to file
	FILE *file = fopen(file_path, "wb");
	if (file)
	{
		fwrite(data, 1, data_size, file);
		fclose(file);
		DbgPrint("Successfully wrote file: %s\n", file_path);
		return 1;
	}
	else
	{
		DbgPrint("Failed to write file: %s\n", file_path);
		return 0;
	}
}

// Function to check if a file exists
bool file_exists(const std::string &file_path)
{
	std::ifstream file(file_path.c_str());
	return file.good();
}

// Function to read a file's contents into a string
std::string read_file_to_string(const std::string &file_path)
{
	std::ifstream file(file_path, std::ios::binary);
	if (!file)
	{
		DbgPrint("read_file_to_string: Failed to open file: %s\n", file_path.c_str());
		return "";
	}

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	return content;
}

namespace game
{
	uint32_t XBOX_360_TITLE_ID = 0x415607E6;

	namespace mp
	{
		typedef void (*Load_MapEntsPtr_t)();
		Load_MapEntsPtr_t Load_MapEntsPtr = reinterpret_cast<Load_MapEntsPtr_t>(0x822A9648);

		struct MapEnts
		{
			const char *name;
			char *entityString;
			int numEntityChars;
		};

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
				std::string file_path = "game:\\dump\\";
				file_path += mapEnts->name;
				file_path += ".ents";										 //  iw4x naming convention
				std::replace(file_path.begin(), file_path.end(), '/', '\\'); // Replace forward slashes with backslashes
				write_file_to_disk(file_path.c_str(), mapEnts->entityString, mapEnts->numEntityChars);

				// Load map ents from file
				// Path to check for existing entity file
				std::string raw_file_path = "game:\\raw\\";
				raw_file_path += mapEnts->name;
				raw_file_path += ".ents";											 // IW4x naming convention
				std::replace(raw_file_path.begin(), raw_file_path.end(), '/', '\\'); // Replace forward slashes with backslashes

				// If the file exists, replace entityString
				if (file_exists(raw_file_path))
				{
					DbgPrint("Found entity file: %s\n", raw_file_path.c_str());
					std::string new_entity_string = read_file_to_string(raw_file_path);
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

		void init()
		{
			XNotifyQueueUI(0, 0, XNOTIFY_SYSTEM, L"iw3 xenon mp", nullptr);

			Load_MapEntsPtr_Detour = Detour(Load_MapEntsPtr, Load_MapEntsPtr_Hook);
			Load_MapEntsPtr_Detour.Install();
		}
	}

	namespace sp
	{
		void init()
		{
			XNotifyQueueUI(0, 0, XNOTIFY_SYSTEM, L"iw3 xenon sp", nullptr);
		}
	}

}

void init()
{
	if (strcmp((char *)0x82032AC4, "multiplayer") == 0)
		game::mp::init();
	else
		game::sp::init();
}

void monitor_title_id()
{
	for (;;)
	{
		if (XamGetCurrentTitleId() == game::XBOX_360_TITLE_ID)
		{
			init();
			break;
		}
		else
			Sleep(100);
	}
}

int DllMain(HANDLE hModule, DWORD reason, void *pReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		ExCreateThread(nullptr, 0, nullptr, nullptr, reinterpret_cast<PTHREAD_START_ROUTINE>(monitor_title_id), nullptr, 2);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}
