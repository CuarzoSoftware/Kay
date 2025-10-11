#ifndef CZ_AKVIBRANCYEVENT_H
#define CZ_AKVIBRANCYEVENT_H

#include <CZ/AK/AK.h>
#include <CZ/Core/Events/CZEvent.h>
#include <CZ/AK/AKVibrancy.h>

class CZ::AKVibrancyEvent : public CZEvent
{
public:
    CZ_EVENT_DECLARE_COPY

    AKVibrancyEvent(AKVibrancyState state) noexcept : CZEvent(Type::Vibrancy),
        state(state) {}

    AKVibrancyState state;
};
#endif // CZ_AKVIBRANCYEVENT_H
