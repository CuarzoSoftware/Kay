#ifndef AKPOINTERSWIPEENDEVENT_H
#define AKPOINTERSWIPEENDEVENT_H

#include <AK/events/AKPointerEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Pointer swipe end gesture event.
 * @ingroup AKEvents
 */
class AK::AKPointerSwipeEndEvent final : public AKPointerEvent
{
public:
    AKEVENT_DECLARE_COPY

    /**
     * @brief Constructs an AKPointerSwipeEndEvent object.
     *
     * @param fingers The number of fingers involved in the swipe gesture.
     * @param cancelled Indicates whether the gesture was cancelled.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKPointerSwipeEndEvent(UInt32 fingers = 0, bool cancelled = false,
                                 UInt32 serial = AKTime::nextSerial(), UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKPointerEvent(PointerSwipeEnd, serial, ms, us, device),
        m_fingers(fingers),
        m_cancelled(cancelled)
    {}

    /**
     * @brief Sets the number of fingers involved in the swipe gesture.
     */
    void setFingers(UInt32 fingers) noexcept
    {
        m_fingers = fingers;
    }

    /**
     * @brief Gets the number of fingers involved in the swipe gesture.
     */
    UInt32 fingers() const noexcept
    {
        return m_fingers;
    }

    /**
     * @brief Sets whether the gesture was cancelled.
     */
    void setCancelled(bool cancelled) noexcept
    {
        m_cancelled = cancelled;
    }

    /**
     * @brief Gets whether the gesture was cancelled.
     */
    bool cancelled() const noexcept
    {
        return m_cancelled;
    }

protected:
    UInt32 m_fingers;
    bool m_cancelled;
};

#endif // AKPOINTERSWIPEENDEVENT_H
