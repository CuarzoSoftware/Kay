#ifndef AKVIBRANCYEVENT_H
#define AKVIBRANCYEVENT_H

#include <AK/events/AKWindowEvent.h>
#include <AK/AKVibrancy.h>
#include <AK/AKTime.h>

class AK::AKVibrancyEvent : public AKEvent
{
public:
    AKEVENT_DECLARE_COPY

    /**
     * @brief Event constructor.
     */
    AKVibrancyEvent(VibrancyState vibrancyState, VibrancyStyle vibrancyStyle,
        UInt32 serial = AKTime::nextSerial(),
                       UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us()) noexcept :
        AKEvent(Vibrancy, serial, ms, us),
        m_vibrancyState(vibrancyState),
        m_vibrancyStyle(vibrancyStyle) {}

    VibrancyState vibrancyState() const noexcept { return m_vibrancyState; }
    VibrancyStyle vibrancyStyle() const noexcept { return m_vibrancyStyle; }

    void setVibrancyState(VibrancyState vibrancyState) const noexcept
    {
        m_vibrancyState = vibrancyState;
    }

    void setVibrancyStyle(VibrancyStyle vibrancyStyle) const noexcept
    {
        m_vibrancyStyle = vibrancyStyle;
    }

protected:
    mutable VibrancyState m_vibrancyState;
    mutable VibrancyStyle m_vibrancyStyle;
};
#endif // AKVIBRANCYEVENT_H
