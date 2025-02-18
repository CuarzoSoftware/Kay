#ifndef AKTOUCHUPEVENT_H
#define AKTOUCHUPEVENT_H

#include <AK/events/AKTouchEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Touch up event.
 * @ingroup AKEvents
 */
class AK::AKTouchUpEvent final : public AKTouchEvent
{
public:
    /**
     * @brief Constructs an AKTouchUpEvent object.
     *
     * @param id The ID of the touch point.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKTouchUpEvent(Int32 id = 0, UInt32 serial = AKTime::nextSerial(), UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKTouchEvent(AKEvent::Subtype::Up, serial, ms, us, device),
        m_id(id)
    {}

    /**
     * @brief Sets the ID of the touch point.
     */
    void setId(Int32 id) noexcept
    {
        m_id = id;
    }

    /**
     * @brief Gets the ID of the touch point.
     */
    Int32 id() const noexcept
    {
        return m_id;
    }

protected:
    Int32 m_id;
    friend class AKTouchEvent;
};

#endif // AKTOUCHUPEVENT_H
