#ifndef AKPOINTERLEAVEEVENT_H
#define AKPOINTERLEAVEEVENT_H

#include <include/core/SkPoint.h>
#include <AK/events/AKPointerEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Event generated when a surface or view loses pointer focus.
 */
class AK::AKPointerLeaveEvent final : public AKPointerEvent
{
public:
    /**
     * @brief Constructs an AKPointerLeaveEvent object.
     *
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKPointerLeaveEvent(const SkPoint &pos = SkPoint(0.f, 0.f), UInt32 serial = AKTime::nextSerial(),
                              UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKPointerEvent(AKEvent::Subtype::Leave, serial, ms, us, device),
        m_pos(pos)
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

protected:
    SkPoint m_pos;
};
#endif // AKPOINTERLEAVEEVENT_H
