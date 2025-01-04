#ifndef AKPOINTERMOVEEVENT_H
#define AKPOINTERMOVEEVENT_H

#include <AK/events/AKPointerEvent.h>
#include <include/core/SkPoint.h>
#include <AK/AKTime.h>

/**
 * @brief Pointer movement event.
 */
class AK::AKPointerMoveEvent final : public AKPointerEvent
{
public:
    /**
     * @brief Constructs an AKPointerMoveEvent object.
     *
     * @param delta The movement delta of the pointer.
     * @param deltaUnaccelerated The unaccelerated movement delta of the pointer.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKPointerMoveEvent(const SkPoint &delta = SkPoint(0.f, 0.f), const SkPoint &deltaUnaccelerated = SkPoint(0.f, 0.f),
                             UInt32 serial = AKTime::nextSerial(), UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKPointerEvent(AKEvent::Subtype::Move, serial, ms, us, device),
        m_delta(delta),
        m_deltaUnaccelerated(deltaUnaccelerated)
    {}

    /**
     * @brief Sets the movement delta of the pointer.
     */
    void setDelta(const SkPoint &delta) noexcept
    {
        m_delta = delta;
    }

    /**
     * @brief Sets the movement delta along the x-axis of the pointer.
     */
    void setDx(Float32 dx) noexcept
    {
        m_delta.fX = dx;
    }

    /**
     * @brief Sets the movement delta along the y-axis of the pointer.
     */
    void setDy(Float32 dy) noexcept
    {
        m_delta.fY = dy;
    }

    /**
     * @brief Gets the movement delta of the pointer.
     */
    const SkPoint &delta() const noexcept
    {
        return m_delta;
    }

    /**
     * @brief Sets the unaccelerated movement delta of the pointer.
     */
    void setDeltaUnaccelerated(const SkPoint &deltaUnaccelerated) noexcept
    {
        m_deltaUnaccelerated = deltaUnaccelerated;
    }

    /**
     * @brief Sets the unaccelerated movement delta along the x-axis of the pointer.
     */
    void setDxUnaccelerated(Float32 dx) noexcept
    {
        m_deltaUnaccelerated.fX = dx;
    }

    /**
     * @brief Sets the unaccelerated movement delta along the y-axis of the pointer.
     */
    void setDyUnaccelerated(Float32 dy) noexcept
    {
        m_deltaUnaccelerated.fY = dy;
    }

    /**
     * @brief Gets the unaccelerated movement delta of the pointer.
     */
    const SkPoint &deltaUnaccelerated() const noexcept
    {
        return m_deltaUnaccelerated;
    }

    /**
     * @brief The surface or view local position where the pointer is positioned in surface coordinates.
     */
    mutable SkPoint localPos;

protected:
    SkPoint m_delta;
    SkPoint m_deltaUnaccelerated;    
};

#endif // AKPOINTERMOVEEVENT_H
