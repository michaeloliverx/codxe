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

    static void PlayerCmd_DisablePlayerClipOnTouchingBrushes(scr_entref_t entref);

  private:
    static dvar_s *noclip_brushes;

    static void RegisterDvars();
    static void RestoreBrushContents();
    static void RemoveAllBrushesContents();
    static std::vector<int> ParseSpaceSeparatedInts(const std::string &str);
    static void HandleclipmapChange();
    static void RebuildNoclipBrushesDvar();
};
} // namespace mp
} // namespace qos
