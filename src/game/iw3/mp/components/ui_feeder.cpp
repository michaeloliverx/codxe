#include "pch.h"
#include "ui_feeder.h"

#include <climits>

namespace iw3
{
namespace mp
{
namespace
{
const unsigned int FEEDER_BACKGROUND_COLUMN = 0;
const unsigned int FEEDER_BACKGROUND_END_COLUMN = 1;
const unsigned int FEEDER_NAME_COLUMN = 2;
const unsigned int FEEDER_ACTION_COLUMN = 3;
const char *const FEEDER_ACTION_GLYPH = "\x01";
const char *const FEEDER_BACKGROUND_MATERIAL = "gradient_fadein";
const char *const FEEDER_BACKGROUND_END_MATERIAL = "button_highlight_end";

const char *FitFeederName(const itemDef_s *item, const char *name)
{
    if (!item || !item->typeData || !name || !*name)
        return name ? name : "";

    const listBoxDef_s *list = static_cast<const listBoxDef_s *>(item->typeData);
    if (list->columnCount <= FEEDER_NAME_COLUMN)
        return name;
    const ListBoxColumn &column = list->columns[FEEDER_NAME_COLUMN];
    Font_s *font = item->fontEnum == 2 ? ui_bigFont : UI_GetFontHandle(item->fontEnum, item->textScale);
    if (!font || font->pixelHeight <= 0 || item->textScale <= 0)
        return name;

    // The native listbox aligns the text to its column but never clips it to that width.
    const float scale = item->textScale * 48.0f / font->pixelHeight;
    const float width = column.width - 2.0f;
    const int maxChars = column.maxChars > 0 ? column.maxChars : INT_MAX;
    if (R_TextWidth(name, maxChars, font) * scale <= width)
        return name;

    const char ellipsis[] = "...";
    const int suffixChars = sizeof(ellipsis) - 1;
    const float prefixWidth = width - R_TextWidth(ellipsis, 0, font) * scale;
    if (prefixWidth < 0 || maxChars < suffixChars)
        return "";

    const char *end = name;
    const char *fittedEnd = name;
    int chars = 0;
    while (*end && chars < maxChars - suffixChars)
    {
        int bytes = 0;
        const int letter = SEH_DecodeLetter(end[0], end[1], &bytes);
        if (bytes <= 0 || letter == '\r' || letter == '\n')
            break;
        end += bytes;
        if (letter == '^' && *end >= '0' && *end <= '9')
        {
            ++end;
            continue;
        }
        if (R_TextWidth(name, ++chars, font) * scale > prefixWidth)
            break;
        fittedEnd = end;
    }

    // The listbox consumes the result immediately when it queues the draw.
    static std::string fitted;
    fitted.assign(name, fittedEnd);
    fitted += ellipsis;
    return fitted.c_str();
}
} // namespace

std::map<float, UIFeederCallbacks> UIFeeder::Feeders;
std::map<float, int> UIFeeder::SelectedIndices;
Detour UIFeeder::UI_FeederCount_Detour;
Detour UIFeeder::UI_FeederItemColor_Detour;
Detour UIFeeder::UI_FeederItemText_Detour;
Detour UIFeeder::UI_FeederSelection_Detour;
Detour UIFeeder::Item_ListBox_Scroll_Detour;

void UIFeeder::Add(float feederID, UIFeederGetItemCount_t getItemCount, UIFeederGetItemText_t getItemText,
                   UIFeederSelect_t select)
{
    if (!getItemCount || !getItemText || !select)
        return;

    UIFeederCallbacks callbacks = {getItemCount, getItemText, select};
    Feeders[feederID] = callbacks;
    SelectedIndices[feederID] = 0;
}

void UIFeeder::SetSelectedIndex(float feederID, int index)
{
    SelectedIndices[feederID] = index;
}

int UIFeeder::UI_FeederCount_Hook(int localClientNum, itemDef_s *item, float feederID)
{
    const std::map<float, UIFeederCallbacks>::const_iterator feeder = Feeders.find(feederID);
    if (feeder == Feeders.end())
        return UI_FeederCount_Detour.GetOriginal<UI_FeederCount_t>()(localClientNum, item, feederID);

    const int count = feeder->second.getItemCount();
    if (item && localClientNum >= 0 && localClientNum < 4)
    {
        int index = SelectedIndices[feederID];
        if (index < 0 || index >= count)
        {
            index = 0;
            SetSelectedIndex(feederID, index);
        }

        item->cursorPos[localClientNum] = index;
    }

    return count;
}

const char *UIFeeder::UI_FeederItemText_Hook(int localClientNum, itemDef_s *item, float feederID, int index,
                                             unsigned int column, Material **handle)
{
    const std::map<float, UIFeederCallbacks>::const_iterator feeder = Feeders.find(feederID);
    if (feeder == Feeders.end())
    {
        return UI_FeederItemText_Detour.GetOriginal<UI_FeederItemText_t>()(localClientNum, item, feederID, index,
                                                                           column, handle);
    }

    if (handle)
        *handle = NULL;

    if (column == FEEDER_BACKGROUND_COLUMN || column == FEEDER_BACKGROUND_END_COLUMN)
    {
        const char *materialName =
            column == FEEDER_BACKGROUND_COLUMN ? FEEDER_BACKGROUND_MATERIAL : FEEDER_BACKGROUND_END_MATERIAL;

        if (handle)
            *handle = DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, materialName).material;

        return "";
    }

    if (column == FEEDER_ACTION_COLUMN)
        return SelectedIndices[feederID] == index ? FEEDER_ACTION_GLYPH : "";

    return column == FEEDER_NAME_COLUMN ? FitFeederName(item, feeder->second.getItemText(index)) : "";
}

void UIFeeder::UI_FeederItemColor_Hook(int localClientNum, itemDef_s *item, float feederID, int index, int column,
                                       float *color)
{
    if (Feeders.find(feederID) != Feeders.end())
    {
        if (column == FEEDER_BACKGROUND_COLUMN || column == FEEDER_BACKGROUND_END_COLUMN)
        {
            const bool selected = SelectedIndices[feederID] == index;
            color[0] = 0.9f;
            color[1] = selected ? 0.95f : 0.9f;
            color[2] = 1.0f;
            color[3] = selected ? 0.25f : 0.07f;
        }
        else
        {
            color[0] = 1.0f;
            color[1] = 1.0f;
            color[2] = 1.0f;
            color[3] = 0.9f;
        }

        return;
    }

    UI_FeederItemColor_Detour.GetOriginal<UI_FeederItemColor_t>()(localClientNum, item, feederID, index, column, color);
}

void UIFeeder::UI_FeederSelection_Hook(int localClientNum, float feederID, itemDef_s *item, int index)
{
    const std::map<float, UIFeederCallbacks>::const_iterator feeder = Feeders.find(feederID);
    if (feeder == Feeders.end())
    {
        UI_FeederSelection_Detour.GetOriginal<UI_FeederSelection_t>()(localClientNum, feederID, item, index);
        return;
    }

    if (index < 0 || index >= feeder->second.getItemCount())
        return;

    SetSelectedIndex(feederID, index);
    feeder->second.select(index);
}

void UIFeeder::Item_ListBox_Scroll_Hook(int localClientNum, itemDef_s *item, int max, int scrollMax, int viewMax,
                                        int delta)
{
    if (item && Feeders.find(item->special) != Feeders.end() && localClientNum >= 0 && localClientNum < 4 && max > 0)
    {
        if (delta < 0 && item->cursorPos[localClientNum] == 0)
        {
            Item_ListBox_SetCursorPos(localClientNum, item, viewMax, max);
            return;
        }

        if (delta > 0 && item->cursorPos[localClientNum] == max)
        {
            Item_ListBox_SetCursorPos(localClientNum, item, viewMax, 0);
            return;
        }
    }

    Item_ListBox_Scroll_Detour.GetOriginal<Item_ListBox_Scroll_t>()(localClientNum, item, max, scrollMax, viewMax,
                                                                    delta);
}

UIFeeder::UIFeeder()
{
    UI_FeederCount_Detour = Detour(UI_FeederCount, UI_FeederCount_Hook);
    UI_FeederCount_Detour.Install();

    UI_FeederItemColor_Detour = Detour(UI_FeederItemColor, UI_FeederItemColor_Hook);
    UI_FeederItemColor_Detour.Install();

    UI_FeederItemText_Detour = Detour(UI_FeederItemText, UI_FeederItemText_Hook);
    UI_FeederItemText_Detour.Install();

    UI_FeederSelection_Detour = Detour(UI_FeederSelection, UI_FeederSelection_Hook);
    UI_FeederSelection_Detour.Install();

    Item_ListBox_Scroll_Detour = Detour(Item_ListBox_Scroll, Item_ListBox_Scroll_Hook);
    Item_ListBox_Scroll_Detour.Install();
}
} // namespace mp
} // namespace iw3
