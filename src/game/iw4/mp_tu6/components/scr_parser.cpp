#include "pch.h"
#include "scr_parser.h"

namespace iw4
{
namespace mp_tu6
{
Detour Scr_AddSourceBuffer_Detour;

char *Scr_AddSourceBuffer_Hook(const char *filename, const char *extFilename)
{
    auto callOriginal = [&]()
    { return Scr_AddSourceBuffer_Detour.GetOriginal<decltype(Scr_AddSourceBuffer)>()(filename, extFilename); };

    if (Config::dump_rawfile)
    {
        auto contents = callOriginal();
        // Dump the script to a file
        std::string dumpPath = std::string(DUMP_DIR) + "\\" + extFilename;
        std::replace(dumpPath.begin(), dumpPath.end(), '/', '\\');
        filesystem::write_file_to_disk(dumpPath.c_str(), contents, std::strlen(contents));
        DbgPrint("GSCLoader: Dumped script to %s\n", dumpPath.c_str());
        return contents;
    }

    // Check if mod is active
    std::string modBasePath = Config::GetModBasePath();
    if (modBasePath.empty())
        return callOriginal();

    // Build full path to override file
    std::string overridePath = modBasePath + "\\" + extFilename;
    std::replace(overridePath.begin(), overridePath.end(), '/', '\\');

    FILE *file = fopen(overridePath.c_str(), "rb");
    if (!file)
        return callOriginal();

    fseek(file, 0, SEEK_END);
    const long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize <= 0)
    {
        fclose(file);
        return callOriginal();
    }

    char *buffer = (char *)Hunk_AllocateTempMemoryHighInternal(fileSize + 1);
    if (!buffer)
    {
        fclose(file);
        return callOriginal();
    }

    const size_t bytesRead = fread(buffer, 1, fileSize, file);
    fclose(file);

    if (bytesRead != static_cast<size_t>(fileSize))
        return callOriginal();

    buffer[bytesRead] = '\0';

    DbgPrint("scr_parser: Loaded override script: %s\n", overridePath.c_str());
    return buffer;
}

scr_parser::scr_parser()
{
    Scr_AddSourceBuffer_Detour = Detour(Scr_AddSourceBuffer, Scr_AddSourceBuffer_Hook);
    Scr_AddSourceBuffer_Detour.Install();
}

scr_parser::~scr_parser()
{
    Scr_AddSourceBuffer_Detour.Remove();
}
} // namespace mp_tu6
} // namespace iw4
