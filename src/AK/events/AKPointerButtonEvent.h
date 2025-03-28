#ifndef AKPOINTERBUTTONEVENT_H
#define AKPOINTERBUTTONEVENT_H

#include <AK/events/AKPointerEvent.h>
#include <linux/input-event-codes.h>
#include <AK/AKTime.h>

/**
 * @brief Pointer button event.
 * @ingroup AKEvents
 */
class AK::AKPointerButtonEvent final : public AKPointerEvent
{
public:
    AKEVENT_DECLARE_COPY

    /**
     * @brief Pointer buttons.
     *
     * Enumeration of common pointer buttons.
     *
     * You can find the complete list of pointer button codes in the [`<linux/input-event-codes.h>`](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h) header.
     */
    enum Button : UInt32
    {
        /// Left button
        Left = BTN_LEFT,

        /// Right button
        Right = BTN_RIGHT,

        /// Middle button
        Middle = BTN_MIDDLE,

        /// Side button
        Side = BTN_SIDE,

        /// Extra button
        Extra = BTN_EXTRA,

        /// Forward button
        Forward = BTN_FORWARD,

        /// Back button
        Back = BTN_BACK,

        /// Task button
        Task = BTN_TASK
    };

    /**
     * @brief Pointer button states.
     *
     * Possible states of a pointer button.
     */
    enum State : UInt32
    {
        /// Button released
        Released = 0,

        /// Button pressed
        Pressed = 1
    };

    /**
     * @brief Constructs an AKPointerButtonEvent object.
     *
     * @param button The button associated with the event.
     * @param state The state of the button.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    AKPointerButtonEvent(Button button = Left, State state = Pressed, UInt32 serial = AKTime::nextSerial(),
                               UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us(), AKInputDevice *device = nullptr) noexcept :
        AKPointerEvent(PointerButton, serial, ms, us, device),
        m_button(button),
        m_state(state)
    {}

    /**
     * @brief Sets the button code associated with the event.
     */
    void setButton(Button button) noexcept
    {
        m_button = button;
    }

    /**
     * @brief Gets the button code associated with the event.
     */
    Button button() const noexcept
    {
        return m_button;
    }

    /**
     * @brief Sets the state of the button.
     */
    void setState(State state) noexcept
    {
        m_state = state;
    }

    /**
     * @brief Gets the state of the button.
     */
    State state() const noexcept
    {
        return m_state;
    }

protected:
    Button m_button;
    State m_state;
};

#endif // AKPOINTERBUTTONEVENT_H
