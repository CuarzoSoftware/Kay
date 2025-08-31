#ifndef CZ_AKPOINTER_H
#define CZ_AKPOINTER_H

#include <CZ/AK/AKObject.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/Events/CZPointerEnterEvent.h>
#include <CZ/Events/CZPointerLeaveEvent.h>
#include <CZ/Events/CZPointerButtonEvent.h>
#include <CZ/Events/CZPointerMoveEvent.h>
#include <CZ/Events/CZPointerScrollEvent.h>

class CZ::AKPointer : public AKObject
{
public:
    struct EventHistory
    {
        CZPointerEnterEvent enter;
        CZPointerLeaveEvent leave;
        CZPointerButtonEvent button;
        CZPointerMoveEvent move;
        CZPointerScrollEvent scroll;
    };

    AKScene *windowFocus() const noexcept
    {
        return m_windowFocus;
    }

    const SkPoint &pos() const noexcept
    {
        return m_pos;
    }

    void setPos(const SkPoint &pos) noexcept
    {
        m_pos = pos;
    }

    const EventHistory &history() const noexcept
    {
        return m_history;
    }

protected:
    AKPointer() = default;
    CZ_DISABLE_COPY(AKPointer)

private:
    friend class AKApp;
    friend class AKScene;
    SkPoint m_pos {};
    CZWeak<AKNode> m_focus;
    CZWeak<AKScene> m_windowFocus;
    EventHistory m_history {};
};

#endif // CZ_AKPOINTER_H
