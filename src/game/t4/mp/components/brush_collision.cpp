#include "brush_collision.h"
#include "common.h"

namespace t4
{
    namespace mp
    {
        dvar_s *noclip_brushes = nullptr;
        std::vector<int> brushContents;

        void SaveBrushContents()
        {
            if (!cm->isInUse)
            {
                DbgPrint("SaveBrushContents: cm is not in use\n");
                return;
            }

            brushContents.resize(cm->numBrushes, 0);
            for (int i = 0; i < cm->numBrushes; ++i)
            {
                brushContents[i] = cm->brushes[i].contents;
            }
        }

        void RestoreBrushContents()
        {
            if (!cm->isInUse)
            {
                DbgPrint("RestoreBrushContents: cm is not in use\n");
                return;
            }

            if (brushContents.size() != static_cast<size_t>(cm->numBrushes))
            {
                DbgPrint("RestoreBrushContents: size mismatch - saved: %zu, current: %d\n",
                         brushContents.size(), cm->numBrushes);
                return;
            }

            for (int i = 0; i < cm->numBrushes; ++i)
            {
                cm->brushes[i].contents = brushContents[i];
            }
        }

        void RemoveAllBrushCollision()
        {
            if (!cm->isInUse)
            {
                DbgPrint("RemoveAllBrushCollision: cm is not in use\n");
                return;
            }

            for (int i = 0; i < cm->numBrushes; ++i)
            {
                cm->brushes[i].contents &= ~0x10000; // Disable collision for all brushes
            }
        }

        Detour CM_LoadMap_Detour;
        void CM_LoadMap_Hook(const char *name)
        {
            // Let the clip map load first
            CM_LoadMap_Detour.GetOriginal<decltype(CM_LoadMap)>()(name);

            // Save the contents of the brushes
            SaveBrushContents();
        }

        std::vector<int> ParseSpaceSeparatedInts(const std::string &str)
        {
            std::vector<int> result;
            std::istringstream iss(str);
            int value;

            while (iss >> value)
            {
                result.push_back(value);
            }

            return result;
        }

        void HandleBrushCollisionChange()
        {
            if (R_CheckDvarModified(noclip_brushes))
            {
                CG_GameMessage(0, "noclip_brushes changed");
                // TODO: find out why noclip_brushes->current.string isn't working
                auto value = noclip_brushes->current.string;

                // Empty string means reset to original contents
                if (strcmp(value, "") == 0)
                {
                    RestoreBrushContents();
                    CG_GameMessage(0, "Brush collision reset");
                }
                // "*" means disable collision for all brushes
                else if (strcmp(value, "*") == 0)
                {
                    RemoveAllBrushCollision();
                    CG_GameMessage(0, "Brush collision disabled for all brushes");
                }
                else
                {
                    RestoreBrushContents();
                    auto brushIndices = ParseSpaceSeparatedInts(value);
                    for (size_t i = 0; i < brushIndices.size(); ++i)
                    {
                        auto idx = brushIndices[i];
                        if (idx < 0 || idx >= cm->numBrushes)
                        {
                            DbgPrint("Error: Invalid brush index: %d\n", idx);
                            continue;
                        }
                        cm->brushes[idx].contents &= ~0x10000; // Disable collision
                        // CG_GameMessage(0, va("Disabled brush collision for brush %d", idx));
                    }
                }
            }
        }

        Detour CG_DrawActive_Detour;

        void CG_DrawActive_Hook(int localClientNum)
        {
            HandleBrushCollisionChange();
            CG_DrawActive_Detour.GetOriginal<decltype(CG_DrawActive)>()(localClientNum);
        }

        BrushCollision::BrushCollision()
        {
            DbgPrint("BrushCollision initialized\n");
            Sleep(1000); // Wait for the game to initialize

            noclip_brushes = Dvar_RegisterString("noclip_brushes", "", DVAR_CODINFO, "Space separated list of brushes to disable collision on.");

            CG_DrawActive_Detour = Detour(CG_DrawActive, CG_DrawActive_Hook);
            CG_DrawActive_Detour.Install();

            CM_LoadMap_Detour = Detour(CM_LoadMap, CM_LoadMap_Hook);
            CM_LoadMap_Detour.Install();
        }

        BrushCollision::~BrushCollision()
        {
            DbgPrint("BrushCollision shutdown\n");

            CG_DrawActive_Detour.Remove();
            CM_LoadMap_Detour.Remove();
        }
    }
}
