#ifndef CZ_AKWINDOWBUTTON_H
#define CZ_AKWINDOWBUTTON_H

#include <CZ/AK/Nodes/AKImage.h>

/**
 * @brief macOS-style window control button (close / minimize / maximize).
 * @ingroup AKNodes
 *
 * Displays the themed image for its type() and state() (see
 * AKTheme::windowButtonImage) at the fixed AKTheme::WindowButtonSize. Releasing
 * a left click performs the matching action on the parent window's toplevel role
 * (when built with Marco) and emits onClick. Usually used through
 * AKWindowButtonGroup, which drives the hover/pressed/disabled states.
 */
class CZ::AKWindowButton : public AKImage
{
public:

    /** @brief The action the button represents (determines its themed image). */
    enum Type
    {
        /// Closes the window.
        Close,

        /// Minimizes the window.
        Minimize,

        /// Toggles the maximized state.
        Maximize,

        /// Makes the window fullscreen.
        Fullscreen,

        /// Exits fullscreen.
        UnsetFullscreen,
    };

    /** @brief Interaction state (determines the themed image variant). */
    enum State
    {
        /// Grayed out; clicks are ignored.
        Disabled,

        /// Idle.
        Normal,

        /// Pointer over the button group.
        Hover,

        /// Being pressed.
        Pressed
    };

    /** @brief Constructs a button of the given type (initial state Disabled). */
    AKWindowButton(Type type, AKNode *parent = nullptr) noexcept;

    /**
     * @brief Sets the button type, updating its themed image.
     *
     * @return `true` if the type changed, `false` otherwise.
     */
    bool setType(Type type) noexcept;

    /** @brief The current button type. */
    Type type() const noexcept { return m_type; }

    /**
     * @brief Sets the interaction state, updating its themed image.
     *
     * @return `true` if the state changed, `false` otherwise.
     */
    bool setState(State state) noexcept;

    /** @brief The current interaction state. Defaults to Disabled. */
    State state() const noexcept { return m_state; }

    /** @brief Presses/releases the button; on left-button release triggers the action and onClick. */
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;

    /** @brief Emitted on left-button release while the button is enabled. */
    CZSignal<> onClick;
protected:
    Type m_type;
    State m_state { Disabled };
};

#endif // CZ_AKWINDOWBUTTON_H
