#ifndef AKPOINTERMOVEEVENT_H
#define AKPOINTERMOVEEVENT_H

#include <AK/events/AKPointerEvent.h>
#include <skia/core/SkPoint.h>
#include <AK/AKTime.h>

/**
 * @brief Pointer movement event.
 * @ingroup AKEvents
 */
class AK::AKPointerMoveEvent final : public AKPointerEvent
{
public:
    AKEVENT_DECLARE_COPY

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
    AKPointerMoveEvent(const SkPoint &pos = SkPoint(0.f, 0.f), const SkPoint &delta = SkPoint(0.f, 0.f), const SkPoint &deltaUnaccelerated = SkPoint(0.f, 0.f),
                             UInt32 serial = AKTime::nextSerial(), UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKPointerEvent(PointerMove, serial, ms, us, device),
        m_pos(pos),
        m_delta(delta),
        m_deltaUnaccelerated(deltaUnaccelerated)
    {}

    /**
     * @brief Sets the scene position of the pointer.
     */
    void setPos(const SkPoint &pos) noexcept
    {
        m_pos = pos;
    }

    /**
     * @brief Sets the x scene position of the pointer.
     */
    void setX(Float32 x) noexcept
    {
        m_pos.fX = x;
    }

    /**
     * @brief Sets the y scene position of the pointer.
     */
    void setY(Float32 y) noexcept
    {
        m_pos.fY = y;
    }

    /**
     * @brief Gets the scene position of the pointer.
     */
    const SkPoint &pos() const noexcept
    {
        return m_pos;
    }

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

protected:
    SkPoint m_pos;
    SkPoint m_delta;
    SkPoint m_deltaUnaccelerated;    
};

#endif // AKPOINTERMOVEEVENT_H
