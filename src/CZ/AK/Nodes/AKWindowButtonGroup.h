#ifndef CZ_AKWINDOWBUTTONGROUP_H
#define CZ_AKWINDOWBUTTONGROUP_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKWindowButton.h>

/**
 * @brief macOS-style row of window control buttons.
 * @ingroup AKNodes
 *
 * Packs a close, a minimize and a maximize/fullscreen AKWindowButton in a row
 * with the theme's gap, and drives their shared state: all three highlight
 * together on hover, revert to Normal or Disabled depending on the window's
 * activated state, and the minimize button is disabled while fullscreen. When
 * built with Marco, holding Alt switches the third button from Fullscreen to
 * Maximize, and entering fullscreen turns it into UnsetFullscreen.
 */
class CZ::AKWindowButtonGroup : public AKContainer
{
public:
    /** @brief Constructs the group; the buttons start Normal or Disabled depending on the window's activated state. */
    AKWindowButtonGroup(AKNode *parent) noexcept;

    /** @brief The close button. */
    AKWindowButton closeButton { AKWindowButton::Type::Close, this };

    /** @brief The minimize button (disabled while the window is fullscreen). */
    AKWindowButton minimizeButton { AKWindowButton::Type::Minimize, this };

    /** @brief The fullscreen/maximize button; its type follows the Alt key and the fullscreen state. */
    AKWindowButton maximizeButton { AKWindowButton::Type::Fullscreen, this };
protected:
    /** @brief Scene-level filter that swaps the third button between Fullscreen and Maximize while Alt is held. */
    bool eventFilter(const CZEvent &e, CZObject &o) noexcept override;

    /** @brief Moves the Alt-key event filter from the old scene to the new one. */
    void sceneChangedEvent(const AKSceneChangedEvent &e) override;

    /** @brief Puts the buttons in the Hover state. */
    void pointerEnterEvent(const CZPointerEnterEvent &e) override;

    /** @brief Reverts the buttons to Normal or Disabled depending on the window's activated state. */
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;

    /** @brief Restores the Hover state after a left-button release. */
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;

    /** @brief Tracks window activation and fullscreen, updating the buttons' states and the third button's type. */
    void windowStateEvent(const CZWindowStateEvent &e) override;
};

#endif // CZ_AKWINDOWBUTTONGROUP_H
