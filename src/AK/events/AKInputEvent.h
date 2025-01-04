#ifndef AKINPUTEVENT_H
#define AKINPUTEVENT_H

#include <AK/events/AKEvent.h>

/**
 * @brief Base class for input events.
 */
class AK::AKInputEvent : public AKEvent
{
public:

    /**
     * @brief Sets the input device that originated the event.
     *
     * @param device If `nullptr` is passed, a generic fake input device is assigned.
     */
    void setDevice(AKInputDevice *device) noexcept
    {
        m_device = device;
    }

    /**
     * @brief Gets the input device that originated this event.
     */
    AKInputDevice *device() const noexcept
    {
        return m_device;
    }

protected:
    AKInputEvent(Type type, Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us, AKInputDevice *device) noexcept :
        AKEvent(type, subtype, serial, ms, us)
    {
        setDevice(device);
    }

    AKInputDevice *m_device;
};

#endif // AKINPUTEVENT_H
