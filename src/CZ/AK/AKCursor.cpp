#include <CZ/AK/AKCursor.h>

const char *CZ::CursorToString(AKCursor cursor) noexcept
{
    switch (cursor) {
    case AKCursor::Hidden: return "none";
    case AKCursor::Default: return "default";
    case AKCursor::ContextMenu: return "context-menu";
    case AKCursor::Help: return "help";
    case AKCursor::Pointer: return "pointer";
    case AKCursor::Progress: return "progress";
    case AKCursor::Wait: return "wait";
    case AKCursor::Cell: return "cell";
    case AKCursor::Crosshair: return "crosshair";
    case AKCursor::Text: return "text";
    case AKCursor::VerticalText: return "vertical-text";
    case AKCursor::Alias: return "alias";
    case AKCursor::Copy: return "copy";
    case AKCursor::Move: return "move";
    case AKCursor::NoDrop: return "no-drop";
    case AKCursor::NotAllowed: return "not-allowed";
    case AKCursor::Grab: return "grab";
    case AKCursor::Grabbing: return "grabbing";
    case AKCursor::EResize: return "e-resize";
    case AKCursor::NResize: return "n-resize";
    case AKCursor::NEResize: return "ne-resize";
    case AKCursor::NWResize: return "nw-resize";
    case AKCursor::SResize: return "s-resize";
    case AKCursor::SEResize: return "se-resize";
    case AKCursor::SWResize: return "sw-resize";
    case AKCursor::WResize: return "w-resize";
    case AKCursor::EWResize: return "ew-resize";
    case AKCursor::NSResize: return "ns-resize";
    case AKCursor::NESWResize: return "nesw-resize";
    case AKCursor::NWSEResize: return "nwse-resize";
    case AKCursor::ColResize: return "col-resize";
    case AKCursor::RowResize: return "row-resize";
    case AKCursor::AllScroll: return "all-scroll";
    case AKCursor::ZoomIn: return "zoom-in";
    case AKCursor::ZoomOut: return "zoom-out";
    default: return "default";
    }
}
