#ifndef AKCURSOR_H
#define AKCURSOR_H

namespace AK
{
    /**
     * @enum AKCursor
     * @brief Enumeration for different cursor types.
     */
    enum class AKCursor
    {
        Hidden, /**< Cursor is hidden */
        Default, /**< Default cursor */
        ContextMenu, /**< Context menu is available for the object under the cursor */
        Help, /**< Help is available for the object under the cursor */
        Pointer, /**< Pointer that indicates a link or another interactive element */
        Progress, /**< Indicates progress */
        Wait, /**< Program is busy, user should wait */
        Cell, /**< A cell or set of cells may be selected */
        Crosshair, /**< Simple crosshair */
        Text, /**< Text may be selected */
        VerticalText, /**< Vertical text may be selected */
        Alias, /**< Drag-and-drop: alias of/shortcut to something is to be created */
        Copy, /**< Drag-and-drop: something is to be copied */
        Move, /**< Drag-and-drop: something is to be moved */
        NoDrop, /**< Drag-and-drop: the dragged item cannot be dropped at the current cursor location */
        NotAllowed, /**< Drag-and-drop: the requested action will not be carried out */
        Grab, /**< Drag-and-drop: something can be grabbed */
        Grabbing, /**< Drag-and-drop: something is being grabbed */
        EResize, /**< Resizing: the east border is to be moved */
        NResize, /**< Resizing: the north border is to be moved */
        NEResize, /**< Resizing: the north-east corner is to be moved */
        NWResize, /**< Resizing: the north-west corner is to be moved */
        SResize, /**< Resizing: the south border is to be moved */
        SEResize, /**< Resizing: the south-east corner is to be moved */
        SWResize, /**< Resizing: the south-west corner is to be moved */
        WResize, /**< Resizing: the west border is to be moved */
        EWResize, /**< Resizing: the east and west borders are to be moved */
        NSResize, /**< Resizing: the north and south borders are to be moved */
        NESWResize, /**< Resizing: the north-east and south-west corners are to be moved */
        NWSEResize, /**< Resizing: the north-west and south-east corners are to be moved */
        ColResize, /**< Resizing: the item/column can be resized horizontally */
        RowResize, /**< Resizing: the item/row can be resized vertically */
        AllScroll, /**< Something can be scrolled in any direction */
        ZoomIn, /**< Something can be zoomed in */
        ZoomOut /**< Something can be zoomed out */
    };

const char *cursorToString(AKCursor cursor) noexcept;
}

#endif // AKCURSOR_H
