#ifndef AKPOINTERLEAVEEVENT_H
#define AKPOINTERLEAVEEVENT_H

#include <AK/events/AKPointerEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Event generated when a surface or view loses pointer focus.
 */
class AK::AKPointerLeaveEvent final : public AKPointerEvent
{
public:
    /**
     * @brief Constructs an AKPointerLeaveEvent object.
     *
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKPointerLeaveEvent(UInt32 serial = AKTime::nextSerial(),
                              UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKPointerEvent(AKEvent::Subtype::Leave, serial, ms, us, device)
    {}
};
#endif // AKPOINTERLEAVEEVENT_H
