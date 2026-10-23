#ifndef CZ_AKKEYMAP_H
#define CZ_AKKEYMAP_H

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <CZ/AK/AKObject.h>
#include <CZ/Core/Events/CZKeyboardEnterEvent.h>
#include <CZ/Core/Events/CZKeyboardKeyEvent.h>
#include <CZ/Core/Events/CZKeyboardModifiersEvent.h>
#include <CZ/Core/Events/CZKeyboardLeaveEvent.h>
#include <CZ/Core/CZWeak.h>

/**
 * @brief Global keyboard state.
 *
 * Owned by AKApp (see AKApp::keyboard() or the CZ::akKeyboard() shortcut), it keeps
 * the most recent keyboard events dispatched by Kay and the scene currently holding
 * keyboard focus. AKApp forwards keyboard events to the focused scene, which then
 * dispatches them to its focused nodes.
 *
 * @ingroup AKInput
 */
class CZ::AKKeyboard : public AKObject
{
public:
    /**
     * @brief Most recent keyboard events handled by Kay.
     */
    struct EventHistory
    {
        /// The most recent keyboard enter event.
        CZKeyboardEnterEvent enter;

        /// The most recent keyboard key event.
        CZKeyboardKeyEvent key;

        /// The most recent keyboard modifiers event.
        CZKeyboardModifiersEvent modifiers;

        /// The most recent keyboard leave event.
        CZKeyboardLeaveEvent leave;
    };

    /** @brief Most recent keyboard event history. */
    const EventHistory &history() const noexcept { return m_history; }

    /**
     * @brief Returns the scene currently holding keyboard focus.
     *
     * This is the last scene that received a CZKeyboardEnterEvent.
     * Keyboard events are only forwarded to the focused scene.
     *
     * @see onFocusChanged
     *
     * @return The focused AKScene, or `nullptr` if none.
     */
    AKScene *focus() const noexcept { return m_focus; }

    /** @brief Emitted when the focused scene changes (see focus()). */
    CZSignal<> onFocusChanged;

protected:
    AKKeyboard() noexcept;

private:
    friend class AKApp;
    friend class AKScene;
    CZWeak<AKScene> m_focus;
    EventHistory m_history {};
};

#endif // CZ_AKKEYMAP_H
