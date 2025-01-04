#ifndef AKPOINTERSWIPEUPDATEEVENT_H
#define AKPOINTERSWIPEUPDATEEVENT_H

#include <AK/events/AKPointerEvent.h>
#include <include/core/SkPoint.h>
#include <AK/AKTime.h>

/**
 * @brief Pointer swipe update gesture event.
 */
class AK::AKPointerSwipeUpdateEvent final : public AKPointerEvent
{
public:
    /**
     * @brief Constructs an AKPointerSwipeUpdateEvent object.
     *
     * @param fingers The number of fingers involved in the swipe gesture.
     * @param delta The change in position of the swipe gesture.
     * @param deltaUnaccelerated The unaccelerated change in position of the swipe gesture.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKPointerSwipeUpdateEvent(UInt32 fingers = 0, const SkPoint &delta = SkPoint(0.f, 0.f), const SkPoint &deltaUnaccelerated = SkPoint(0.f, 0.f),
                                    UInt32 serial = AKTime::nextSerial(), UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKPointerEvent(AKEvent::Subtype::SwipeUpdate, serial, ms, us, device),
        m_fingers(fingers),
        m_delta(delta),
        m_deltaUnaccelerated(deltaUnaccelerated)
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
     * @brief Sets the change in position of the swipe gesture.
     */
    void setDelta(const SkPoint &delta) noexcept
    {
        m_delta = delta;
    }

    /**
     * @brief Sets the change in position of the swipe gesture along the X-axis.
     */
    void setDx(Float32 dx) noexcept
    {
        m_delta.fX = dx;
    }

    /**
     * @brief Sets the change in position of the swipe gesture along the Y-axis.
     */
    void setDy(Float32 dy) noexcept
    {
        m_delta.fY = dy;
    }

    /**
     * @brief Gets the change in position of the swipe gesture.
     */
    const SkPoint &delta() const noexcept
    {
        return m_delta;
    }

    /**
     * @brief Sets the unaccelerated change in position of the swipe gesture.
     */
    void setDeltaUnaccelerated(const SkPoint &deltaUnaccelerated) noexcept
    {
        m_deltaUnaccelerated = deltaUnaccelerated;
    }

    /**
     * @brief Sets the unaccelerated change in position of the swipe gesture along the X-axis.
     */
    void setDxUnaccelerated(Float32 dx) noexcept
    {
        m_deltaUnaccelerated.fX = dx;
    }

    /**
     * @brief Sets the unaccelerated change in position of the swipe gesture along the Y-axis.
     */
    void setDyUnaccelerated(Float32 dy) noexcept
    {
        m_deltaUnaccelerated.fY = dy;
    }

    /**
     * @brief Gets the unaccelerated change in position of the swipe gesture.
     */
    const SkPoint &deltaUnaccelerated() const noexcept
    {
        return m_deltaUnaccelerated;
    }

protected:
    UInt32 m_fingers;
    SkPoint m_delta;
    SkPoint m_deltaUnaccelerated;
};

#endif // AKPOINTERSWIPEUPDATEEVENT_H
