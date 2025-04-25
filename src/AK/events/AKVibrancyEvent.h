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
    AKVibrancyEvent(AKVibrancyState vibrancyState, AKVibrancyStyle vibrancyStyle,
        UInt32 serial = AKTime::nextSerial(),
                       UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us()) noexcept :
        AKEvent(Vibrancy, serial, ms, us),
        m_vibrancyState(vibrancyState),
        m_vibrancyStyle(vibrancyStyle) {}

    AKVibrancyState vibrancyState() const noexcept { return m_vibrancyState; }
    AKVibrancyStyle vibrancyStyle() const noexcept { return m_vibrancyStyle; }

    void setVibrancyState(AKVibrancyState vibrancyState) noexcept
    {
        m_vibrancyState = vibrancyState;
    }

    void setVibrancyStyle(AKVibrancyStyle vibrancyStyle) noexcept
    {
        m_vibrancyStyle = vibrancyStyle;
    }

protected:
    AKVibrancyState m_vibrancyState;
    AKVibrancyStyle m_vibrancyStyle;
};
#endif // AKVIBRANCYEVENT_H
