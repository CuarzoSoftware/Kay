#ifndef AKTOUCHEVENT_H
#define AKTOUCHEVENT_H

#include <AK/events/AKInputEvent.h>

/**
 * @brief Base class for touch events.
 * @ingroup AKEvents
 *
 * All touch events share the same AKEvent::Type::Touch type.
 */
class AK::AKTouchEvent : public AKInputEvent
{
public:
    /**
     * @brief Gets the unique identifier of the touch point.
     *
     * @note If the subtype is @ref AKEvent::Subtype::Frame or @ref AKEvent::Subtype::Cancel -1 is returned.
     */
    Int32 id() const noexcept;

protected:
    AKTouchEvent(Type type, UInt32 serial, UInt32 ms, UInt64 us, AKInputDevice *device) noexcept :
        AKInputEvent(type, serial, ms, us, device)
    {}
};

#endif // AKTOUCHEVENT_H
