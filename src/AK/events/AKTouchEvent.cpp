#include <AK/events/AKTouchEvent.h>
#include <AK/events/AKTouchDownEvent.h>
#include <AK/events/AKTouchMoveEvent.h>
#include <AK/events/AKTouchUpEvent.h>

using namespace AK;

Int32 AKTouchEvent::id() const noexcept
{
    switch (type())
    {
    case TouchDown:
        return static_cast<const AKTouchDownEvent*>(this)->m_id;
    case TouchMove:
        return static_cast<const AKTouchMoveEvent*>(this)->m_id;
    case TouchUp:
        return static_cast<const AKTouchUpEvent*>(this)->m_id;
    default:
        return -1;
    }
}
