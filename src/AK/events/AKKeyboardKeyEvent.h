#ifndef AKKEYBOARDKEYEVENT_H
#define AKKEYBOARDKEYEVENT_H

#include <xkbcommon/xkbcommon.h>
#include <AK/events/AKKeyboardEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Keyboard key event.
 * @ingroup AKEvents
 */
class AK::AKKeyboardKeyEvent final : public AKKeyboardEvent
{
public:

    /**
     * @brief Key states.
     *
     * Enum with the possible states of a key.
     */
    enum State : UInt32
    {
        /// The key is not being pressed
        Released = 0,

        /// The key is pressed
        Pressed = 1
    };

    /**
     * @brief Constructor for AKKeyboardKeyEvent.
     *
     * @param keyCode The raw key code.
     * @param state The state of the key (Pressed or Released).
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKKeyboardKeyEvent(UInt32 keyCode = 0, State state = Pressed, UInt32 serial = AKTime::nextSerial(),
                      UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKKeyboardEvent(AKEvent::Subtype::Key, serial, ms, us, device),
        m_key(keyCode),
        m_state(state)
    {}

    /**
     * @brief Sets the raw key code.
     */
    void setKeyCode(UInt32 keyCode) noexcept
    {
        m_key = keyCode;
    }

    /**
     * @brief Gets the raw key code.
     */
    UInt32 keyCode() const noexcept
    {
        return m_key;
    }

    /**
     * @brief Gets the XKB symbol.
     */
    xkb_keysym_t keySymbol() const noexcept;

    /**
     * @brief Gets the string representation of the key.
     */
    const char *keyString() const noexcept;

    /**
     * @brief Sets the state of the key.
     */
    void setState(State state) noexcept
    {
        m_state = state;
    }

    /**
     * @brief Gets the state of the key.
     */
    State state() const noexcept
    {
        return m_state;
    }

protected:
    UInt32 m_key;
    State m_state;    
};
#endif // AKKEYBOARDKEYEVENT_H
