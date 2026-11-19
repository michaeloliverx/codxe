#ifndef CODXE_FEEDER_MENU_INC
#define CODXE_FEEDER_MENU_INC

#define CODXE_FEEDER_ACTION_Y 378

#define CODXE_FEEDER_LIST_STYLE(rowWidth, nameWidth, actionX, nameAlignment) \
            style                       WINDOW_STYLE_FILLED \
            type                        ITEM_TYPE_LISTBOX \
            borderSize                  1 \
            visible                     1 \
            textalignx                  6 \
            textscale                   0.4 \
            textstyle                   ITEM_TEXTSTYLE_SHADOWED \
            forecolor                   1 1 1 0.9 \
            outlinecolor                0 0 0 0 \
            selectBorder                0.8 0.95 1 0 \
            elementwidth                120 \
            elementheight               22 \
            columns                     4 \
                                        0 rowWidth 0 0 \
                                        rowWidth 5 0 0 \
                                        6 nameWidth 64 nameAlignment \
                                        actionX 20 1 0

#endif
