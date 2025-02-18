#ifndef AKKEYBOARDMODIFIERSEVENT_H
#define AKKEYBOARDMODIFIERSEVENT_H

#include <AK/events/AKKeyboardEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Keyboard modifiers event.
 * @ingroup AKEvents
 *
 * Keyboard modifiers events are automatically sent to client surfaces when they acquire keyboard focus.
 */
class AK::AKKeyboardModifiersEvent final : public AKKeyboardEvent
{
public:

    /**
     * @brief Keyboard modifiers.
     */
    struct Modifiers
    {
        /// Active modifiers when physically pressed
        UInt32 depressed = 0;

        /// Hooked modifiers that will be disabled after a non-modifier key is pressed
        UInt32 latched = 0;

        /// Active modifiers until they are pressed again (e.g. the Shift key)
        UInt32 locked = 0;

        /// Group the above states (use this value if the source of a modifier change is not of your interest)
        UInt32 group = 0;
    };

    /**
     * @brief Constructor for AKKeyboardModifiersEvent.
     *
     * @param modifiers The keyboard modifiers to be set.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKKeyboardModifiersEvent(const Modifiers &modifiers = {0, 0, 0, 0},
                                   UInt32 serial = AKTime::nextSerial(), UInt32 ms = AKTime::ms(),
                                   UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept:
        AKKeyboardEvent(AKEvent::Subtype::Modifiers, serial, ms, us, device),
        m_modifiers(modifiers)
    {}

    /**
     * @brief Sets the keyboard modifiers for this event.
     */
    void setModifiers(const Modifiers &modifiers) noexcept
    {
        m_modifiers = modifiers;
    }

    /**
     * @brief Gets the keyboard modifiers for this event.
     */
    const Modifiers &modifiers() const noexcept
    {
        return m_modifiers;
    }

protected:
    Modifiers m_modifiers;
};

#endif // AKKEYBOARDMODIFIERSEVENT_H
