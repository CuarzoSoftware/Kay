#ifndef CZ_AKVIBRANCYEVENT_H
#define CZ_AKVIBRANCYEVENT_H

#include <CZ/AK/AK.h>
#include <CZ/Core/Events/CZEvent.h>
#include <CZ/AK/AKVibrancy.h>

/**
 * @brief Notifies nodes that the vibrancy (background blur) state changed.
 *
 * Sent to a node tree when vibrancy becomes enabled or disabled, e.g. by Marco's
 * MSurface when the compositor confirms or revokes background blur support, so
 * nodes such as MVibrancyView can switch between vibrant and opaque styling.
 *
 * @ingroup AKEvents
 */
class CZ::AKVibrancyEvent : public CZEvent
{
public:
    CZ_EVENT_DECLARE_COPY

    /**
     * @brief Constructs a vibrancy event.
     *
     * @param state The new vibrancy state.
     */
    AKVibrancyEvent(AKVibrancyState state) noexcept : CZEvent(Type::Vibrancy),
        state(state) {}

    /// The new vibrancy state.
    AKVibrancyState state;
};
#endif // CZ_AKVIBRANCYEVENT_H
