#ifndef AKPOINTERPINCHENDEVENT_H
#define AKPOINTERPINCHENDEVENT_H

#include <AK/events/AKPointerEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Pointer pinch end gesture event.
 * @ingroup AKEvents
 */
class AK::AKPointerPinchEndEvent final : public AKPointerEvent
{
public:

    /**
     * @brief Constructs an AKPointerPinchEndEvent object.
     *
     * @param fingers The number of fingers involved in the pinch gesture.
     * @param cancelled Indicates whether the pinch gesture was cancelled.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKPointerPinchEndEvent(UInt32 fingers = 0, bool cancelled = false,
                                 UInt32 serial = AKTime::nextSerial(), UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKPointerEvent(AKEvent::Subtype::PinchEnd, serial, ms, us, device),
        m_fingers(fingers),
        m_cancelled(cancelled)
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

    /**
     * @brief Sets whether the pinch gesture was cancelled.
     */
    void setCancelled(bool cancelled) noexcept
    {
        m_cancelled = cancelled;
    }

    /**
     * @brief Gets whether the pinch gesture was cancelled.
     */
    bool cancelled() const noexcept
    {
        return m_cancelled;
    }

protected:
    UInt32 m_fingers;
    bool m_cancelled;
};

#endif // AKPOINTERPINCHENDEVENT_H
