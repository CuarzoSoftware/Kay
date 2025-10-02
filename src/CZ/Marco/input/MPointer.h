#ifndef MPOINTER_H
#define MPOINTER_H

#include <CZ/Marco/Marco.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/Core/CZCursorShape.h>
#include <CZ/AK/Input/AKPointer.h>
#include <CZ/Events/CZPointerEnterEvent.h>
#include <CZ/Events/CZPointerMoveEvent.h>
#include <CZ/Events/CZPointerLeaveEvent.h>
#include <CZ/Events/CZPointerButtonEvent.h>
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>
#include <unordered_set>
#include <unordered_map>

class CZ::MPointer : public AKPointer
{
public:
    MPointer() = default;

    struct EventHistory
    {
        CZPointerEnterEvent enter;
        CZPointerMoveEvent move;
        CZPointerLeaveEvent leave;
        CZPointerButtonEvent button;
        CZPointerScrollEvent scroll;
    };

    CZCursorShape cursor() const noexcept { return m_cursor; };
    void setCursor(CZCursorShape cursor) noexcept;
    MSurface *focus() const noexcept { return m_focus; };
    const EventHistory &eventHistory() const noexcept { return m_eventHistory; };
    const std::unordered_set<UInt32> pressedButtons() const noexcept { return m_pressedButtons; };
    bool isButtonPressed(UInt32 button) const noexcept
    {
        return m_pressedButtons.contains(button);
    }

private:
    friend class MApplication;
    CZCursorShape findNonDefaultCursor(AKNode *node) const noexcept;
    EventHistory m_eventHistory;
    CZPointerScrollEvent m_framedScrollEvent;
    std::unordered_set<UInt32> m_pressedButtons;
    CZWeak<MSurface> m_focus;
    wl_surface *m_cursorSurface { nullptr };
    wl_cursor_theme *m_cursorTheme { nullptr };
    std::unordered_map<CZCursorShape, wl_cursor*> m_cursors;
    CZCursorShape m_cursor { CZCursorShape::Default };
    bool m_forceCursorUpdate { true };
    bool m_hasPendingAxisEvent { false };
};

#endif // MPOINTER_H
