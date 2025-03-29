#ifndef AKPOINTER_H
#define AKPOINTER_H

#include <AK/AKObject.h>
#include <AK/AKWeak.h>
#include <AK/events/AKPointerEnterEvent.h>
#include <AK/events/AKPointerLeaveEvent.h>
#include <AK/events/AKPointerButtonEvent.h>
#include <AK/events/AKPointerMoveEvent.h>
#include <AK/events/AKPointerScrollEvent.h>

class AK::AKPointer : public AKObject
{
public:
    struct EventHistory
    {
        AKPointerEnterEvent enter;
        AKPointerLeaveEvent leave;
        AKPointerButtonEvent button;
        AKPointerMoveEvent move;
        AKPointerScrollEvent scroll;
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
    AKCLASS_NO_COPY(AKPointer)

private:
    friend class AKApplication;
    friend class AKScene;
    SkPoint m_pos { 0, 0 };
    AKWeak<AKNode> m_focus;
    AKWeak<AKScene> m_windowFocus;
    EventHistory m_history;
};

#endif // AKPOINTER_H
