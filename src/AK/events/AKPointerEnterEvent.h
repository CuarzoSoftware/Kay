#ifndef AKPOINTERENTEREVENT_H
#define AKPOINTERENTEREVENT_H

#include <include/core/SkPoint.h>
#include <AK/events/AKPointerEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Event generated when a surface or view gains pointer focus.
 */
class AK::AKPointerEnterEvent final : public AKPointerEvent
{
public:
    /**
     * @brief Constructs an AKPointerEnterEvent object.
     *
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKPointerEnterEvent(UInt32 serial = AKTime::nextSerial(),
                              UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKPointerEvent(AKEvent::Subtype::Enter, serial, ms, us, device)
    {}

    /**
     * @brief The surface or view local position where the pointer entered in surface coordinates.
     */
    mutable SkPoint localPos;
};

#endif // AKPOINTERENTEREVENT_H
