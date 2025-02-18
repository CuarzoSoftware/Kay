#ifndef AKKEYBOARDLEAVEEVENT_H
#define AKKEYBOARDLEAVEEVENT_H

#include <AK/events/AKKeyboardEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Event generated when a surface or view loses keyboard focus.
 * @ingroup AKEvents
 */
class AK::AKKeyboardLeaveEvent final : public AKKeyboardEvent
{
public:
    /**
     * @brief Constructor for AKKeyboardLeaveEvent.
     *
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKKeyboardLeaveEvent(UInt32 serial = AKTime::nextSerial(), UInt32 ms = AKTime::ms(),
                        UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept:
        AKKeyboardEvent(AKEvent::Subtype::Leave, serial, ms, us, device)
    {}
};

#endif // AKKEYBOARDLEAVEEVENT_H
