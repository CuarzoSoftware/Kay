#ifndef AKTOUCHCANCELEVENT_H
#define AKTOUCHCANCELEVENT_H

#include <AK/events/AKTouchEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Touch cancel event.
 */
class AK::AKTouchCancelEvent final : public AKTouchEvent
{
public:
    /**
     * @brief Constructs an AKTouchCancelEvent object.
     *
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKTouchCancelEvent(UInt32 serial = AKTime::nextSerial(),
                             UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKTouchEvent(AKEvent::Subtype::Cancel, serial, ms, us, device)
    {}
};

#endif // AKTOUCHCANCELEVENT_H
