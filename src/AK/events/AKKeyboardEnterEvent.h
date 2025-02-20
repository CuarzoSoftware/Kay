#ifndef AKKEYBOARDENTEREVENT_H
#define AKKEYBOARDENTEREVENT_H

#include <AK/events/AKKeyboardEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Event generated when a surface or view gains keyboard focus.
 */
class AK::AKKeyboardEnterEvent final : public AKKeyboardEvent
{
public:

    /**
     * @brief Constructor for AKKeyboardEnterEvent.
     *
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKKeyboardEnterEvent(UInt32 serial = AKTime::nextSerial(), UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKKeyboardEvent(KeyboardEnter, serial, ms, us, device)
    {}
};

#endif // AKKEYBOARDENTEREVENT_H
