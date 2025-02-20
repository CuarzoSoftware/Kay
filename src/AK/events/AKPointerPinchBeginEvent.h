#ifndef AKPOINTERPINCHBEGINEVENT_H
#define AKPOINTERPINCHBEGINEVENT_H

#include <AK/events/AKPointerEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Pointer pinch begin gesture event.
 * @ingroup AKEvents
 */
class AK::AKPointerPinchBeginEvent final : public AKPointerEvent
{
public:
    AKEVENT_DECLARE_COPY

    /**
     * @brief Constructs an AKPointerPinchBeginEvent object.
     *
     * @param fingers The number of fingers involved in the pinch gesture.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKPointerPinchBeginEvent(UInt32 fingers = 0,
                                   UInt32 serial = AKTime::nextSerial(), UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKPointerEvent(PointerPinchBegin, serial, ms, us, device),
        m_fingers(fingers)
    {}

    /**
     * @brief Sets the number of fingers involved in the pinch gesture.
     */
    void setFingers(UInt32 fingers) noexcept
    {
        m_fingers = fingers;
    }

    /**
     * @brief Gets the number of fingers involved in the pinch gesture.
     */
    UInt32 fingers() const noexcept
    {
        return m_fingers;
    }

protected:
    UInt32 m_fingers;
};

#endif // AKPOINTERPINCHBEGINEVENT_H
