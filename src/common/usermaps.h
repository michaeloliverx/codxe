#pragma once

#include <string>
#include <vector>
#include "arena.h"

namespace usermaps
{
struct Credits
{
    std::string author;
    std::string revision;
};

struct Entry
{
    std::string name;
    std::string displayName;
    std::string description;
    std::string previewPath;
    Credits credits;
    std::vector<std::string> tags;
};

// A browser view of the catalog. Indices in the view resolve to the full list.
class TagFilter
{
  public:
    TagFilter() : selectedTag(0) {}
    void Reset(const std::vector<Entry> &maps);
    void SelectTag(int index);
    bool HasTags() const;
    int TagCount() const;
    int SelectedTag() const;
    const char *TagText(int index) const;
    std::string Prompt() const;
    int MapCount() const;
    int MapIndex(int row) const;
    int RowForMap(int index) const;

  private:
    struct Option
    {
        std::string label;
        std::vector<int> maps;
    };
    std::vector<Option> options;
    int selectedTag;
};

const char *GetDirectory();
bool IsSafeName(const char *name);
bool IsMultiplayerName(const char *name);
std::string FastfilePath(const char *zoneName);
bool ReadMetadata(const std::string &directory, const std::string &map, std::string &name, std::string &description);
bool ReadMetadata(const std::string &directory, const std::string &map, arena::Fields &fields);
Credits ReadCredits(const arena::Fields &fields);
std::vector<std::string> ReadTags(const arena::Fields &fields);
std::vector<Entry> ScanMultiplayer();
} // namespace usermaps
