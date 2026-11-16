#ifndef CZ_AKWINDOWBUTTONGROUP_H
#define CZ_AKWINDOWBUTTONGROUP_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKWindowButton.h>

/**
 * @brief macOS-style row of window control buttons (traffic lights).
 * @ingroup AKNodes
 *
 * Packs a close, a minimize and a maximize/fullscreen AKWindowButton in a row with the theme's gap,
 * and drives their looks like macOS (and Crystals' server-side decorations):
 *
 * - Colored while the window is activated, gray otherwise.
 * - Hovering the group (a button or the gaps between them) shows the symbols on all three, colored
 *   even if the window isn't activated.
 * - A button pressed looks pressed only while the pointer is on it; releasing it there is a click
 *   (the button's AKWindowButton::onClick), anywhere else nothing.
 * - The third button makes the window fullscreen, toggles maximized while Alt is held, and exits
 *   fullscreen while fullscreen (its type()). Minimizing is disabled while fullscreen.
 *
 * It only shows the buttons and reports clicks: performing the actions (and hiding the group when
 * the window doesn't need it, e.g. fullscreen or server-side decorated) is up to the toolkit
 * showing the window, e.g. Marco's MToplevel.
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

    /** @brief Updates the buttons' types and states (see the class description). */
    void updateButtons() noexcept;

protected:
    /** @brief Scene-level filter that tracks the Alt key (switches the third button to Maximize). */
    bool eventFilter(const CZEvent &e, CZObject &o) noexcept override;

    /** @brief Moves the Alt-key event filter from the old scene to the new one. */
    void sceneChangedEvent(const AKSceneChangedEvent &e) override;

    void pointerEnterEvent(const CZPointerEnterEvent &e) override;
    void pointerMoveEvent(const CZPointerMoveEvent &e) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;

    /** @brief Tracks window activation and fullscreen. */
    void windowStateEvent(const CZWindowStateEvent &e) override;

private:
    /// The button under the pointer (nullptr over a gap or outside).
    AKWindowButton *buttonAtPointer() const noexcept;
    bool pointerInside() const noexcept;
    void updatePointer() noexcept;
    bool fullscreen() const noexcept;

    bool m_hover { false };
    bool m_alt { false };
    AKWindowButton *m_pressed { nullptr };
    bool m_pressedInside { false };
};

#endif // CZ_AKWINDOWBUTTONGROUP_H
