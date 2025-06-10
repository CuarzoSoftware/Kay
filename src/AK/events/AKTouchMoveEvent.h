#ifndef AKTOUCHMOVEEVENT_H
#define AKTOUCHMOVEEVENT_H

#include <AK/events/AKTouchEvent.h>
#include <skia/core/SkPoint.h>
#include <AK/AKTime.h>

/**
 * @brief Touch move event.
 * @ingroup AKEvents
 */
class AK::AKTouchMoveEvent final : public AKTouchEvent
{
public:
    AKEVENT_DECLARE_COPY

    /**
     * @brief Constructs an AKTouchMoveEvent object.
     *
     * @param id The ID of the touch point.
     * @param pos The position of the touch point on the touch device [0, 1].
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKTouchMoveEvent(Int32 id = 0, const SkPoint &pos = SkPoint(0.f, 0.f), UInt32 serial = AKTime::nextSerial(),
                           UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKTouchEvent(TouchMove, serial, ms, us, device),
        m_id(id),
        m_pos(pos)
    {}

    /**
     * @brief Sets the position of the touch point.
     */
    void setPos(const SkPoint &pos) noexcept
    {
        m_pos = pos;
    }

    /**
     * @brief Sets the position of the touch point.
     */
    void setPos(Float32 x, Float32 y) noexcept
    {
        m_pos.fX = x;
        m_pos.fY = y;
    }

    /**
     * @brief Sets the x-coordinate of the touch point position.
     */
    void setX(Float32 x) noexcept
    {
        m_pos.fX = x;
    }

    /**
     * @brief Sets the y-coordinate of the touch point position.
     */
    void setY(Float32 y) noexcept
    {
        m_pos.fY = y;
    }

    /**
     * @brief Gets the position of the touch point.
     *
     * @note The position is typically normalized to the range [0, 1] for both axes.
     */
    const SkPoint &pos() const noexcept
    {
        return m_pos;
    }

    /**
     * @brief Sets the unique identifier of the touch point.
     */
    void setId(Int32 id) noexcept
    {
        m_id = id;
    }

    /**
     * @brief Gets the unique identifier of the touch point.
     */
    Int32 id() const noexcept
    {
        return m_id;
    }

    /**
     * @brief The surface or view local position where the touch point is positioned in surface coordinates.
     */
    mutable SkPoint localPos;

protected:
    Int32 m_id;
    SkPoint m_pos;
    friend class AKTouchEvent;
};

#endif // AKTOUCHMOVEEVENT_H
