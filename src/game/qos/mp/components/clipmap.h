#pragma once

#include "pch.h"

namespace qos
{
namespace mp
{
class clipmap : public Module
{
  public:
    clipmap();
    ~clipmap();

    const char *get_name() override
    {
        return "clipmap";
    };

  private:
    static dvar_s *noclip_brushes;
    static std::vector<int> brushContents;

    static void RegisterDvars();
    static void SaveBrushContents();
    static void RestoreBrushContents();
    static void RemoveAllBrushesContents();
    static std::vector<int> ParseSpaceSeparatedInts(const std::string &str);
    static void HandleclipmapChange();
    static void RebuildNoclipBrushesDvar();
    static void PlayerCmd_DisablePlayerClipOnTouchingBrushes(scr_entref_t entref);
};
} // namespace mp
} // namespace qos
