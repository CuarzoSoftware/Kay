#ifndef MPOINTER_H
#define MPOINTER_H

#include <Marco/Marco.h>
#include <AK/AKCursor.h>
#include <AK/AKWeak.h>
#include <AK/input/AKPointer.h>
#include <AK/events/AKPointerEnterEvent.h>
#include <AK/events/AKPointerMoveEvent.h>
#include <AK/events/AKPointerLeaveEvent.h>
#include <AK/events/AKPointerButtonEvent.h>
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>
#include <unordered_set>
#include <unordered_map>

class AK::MPointer : public AKPointer
{
public:
    MPointer() = default;

    struct EventHistory
    {
        AKPointerEnterEvent enter;
        AKPointerMoveEvent move;
        AKPointerLeaveEvent leave;
        AKPointerButtonEvent button;
    };

    AKCursor cursor() const noexcept { return m_cursor; };
    void setCursor(AKCursor cursor) noexcept;
    MSurface *focus() const noexcept { return m_focus; };
    const EventHistory &eventHistory() const noexcept { return m_eventHistory; };
    const std::unordered_set<UInt32> pressedButtons() const noexcept { return m_pressedButtons; };
    bool isButtonPressed(UInt32 button) const noexcept
    {
        return m_pressedButtons.contains(button);
    }

private:
    friend class MApplication;
    AKCursor findNonDefaultCursor(AKNode *node) const noexcept;
    EventHistory m_eventHistory;
    std::unordered_set<UInt32> m_pressedButtons;
    AKWeak<MSurface> m_focus;
    wl_surface *m_cursorSurface { nullptr };
    wl_cursor_theme *m_cursorTheme { nullptr };
    std::unordered_map<AKCursor, wl_cursor*> m_cursors;
    AKCursor m_cursor { AKCursor::Default };
    bool m_forceCursorUpdate { true };
};

#endif // MPOINTER_H
