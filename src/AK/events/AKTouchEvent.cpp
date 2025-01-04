#include <AK/events/AKTouchEvent.h>
#include <AK/events/AKTouchDownEvent.h>
#include <AK/events/AKTouchMoveEvent.h>
#include <AK/events/AKTouchUpEvent.h>

using namespace AK;

Int32 AKTouchEvent::id() const noexcept
{
    if (subtype() == AKEvent::Subtype::Down)
        return static_cast<const AKTouchDownEvent*>(this)->m_id;
    else if (subtype() == AKEvent::Subtype::Move)
        return static_cast<const AKTouchMoveEvent*>(this)->m_id;
    else if (subtype() == AKEvent::Subtype::Up)
        return static_cast<const AKTouchUpEvent*>(this)->m_id;

    return -1;
}
