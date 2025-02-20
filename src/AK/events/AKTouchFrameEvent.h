#ifndef AKTOUCHFRAMEEVENT_H
#define AKTOUCHFRAMEEVENT_H

#include <AK/events/AKTouchEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Represents a touch frame event.
 * @ingroup AKEvents
 *
 * This event marks a set of touch events that belong logically together.
 */
class AK::AKTouchFrameEvent final : public AKTouchEvent
{
public:
    AKEVENT_DECLARE_COPY

    /**
     * @brief Constructs an AKTouchFrameEvent object.
     *
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKTouchFrameEvent(UInt32 serial = AKTime::nextSerial(),
                            UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKTouchEvent(TouchFrame, serial, ms, us, device)
    {}
};

#endif // AKTOUCHFRAMEEVENT_H
