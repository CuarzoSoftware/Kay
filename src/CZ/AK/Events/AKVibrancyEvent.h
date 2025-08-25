#ifndef CZ_AKVIBRANCYEVENT_H
#define CZ_AKVIBRANCYEVENT_H

#include <CZ/AK/AK.h>
#include <CZ/Events/CZEvent.h>
#include <CZ/AK/AKVibrancy.h>

class CZ::AKVibrancyEvent : public CZEvent
{
public:
    CZ_EVENT_DECLARE_COPY

    AKVibrancyEvent(AKVibrancyState state, AKVibrancyStyle style) noexcept : CZEvent(Type::Vibrancy),
        state(state),
        style(style) {}

    AKVibrancyState state;
    AKVibrancyStyle style;
};
#endif // CZ_AKVIBRANCYEVENT_H
