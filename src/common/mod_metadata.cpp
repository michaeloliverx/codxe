#include "pch.h"
#include "mod_metadata.h"

namespace mod_metadata
{
Entry Read(const char *directory, const char *name)
{
    Entry entry;
    entry.name = name;

    arena::Fields fields;
    if (arena::Parse(arena::ReadText(filesystem::JoinPath(directory, "mod.arena")), fields))
    {
        entry.displayName = arena::ReadPlainLine(fields, "longname", 64);
        entry.description = fields["description"];
        entry.credits = usermaps::ReadCredits(fields);
    }
    if (entry.displayName.empty())
        entry.displayName = name;

    const std::string previewPath = filesystem::JoinPath(directory, "preview.dds");
    if (filesystem::FileExists(previewPath.c_str()))
        entry.previewPath = previewPath;
    return entry;
}
} // namespace mod_metadata
