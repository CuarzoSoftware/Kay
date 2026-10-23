#ifndef CZ_AKSCROLL_H
#define CZ_AKSCROLL_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKScrollBar.h>
#include <CZ/Core/CZLinearAnimation.h>

/**
 * @brief Scrollable container.
 * @ingroup AKNodes
 *
 * Children are redirected to an internal content slot that is moved around to
 * scroll (see setOffset()). Pointer scroll events pan the content: finger input
 * gets kinetic inertia with elastic overscroll at the edges, other sources
 * scroll directly. Arrow keys also scroll (via an event filter installed on the
 * scene) and a pair of auto-hiding overlay AKScrollBar nodes reflects and
 * controls the position.
 */
class CZ::AKScroll : public AKContainer
{
public:
    /** @brief Constructs an empty scroll container. */
    AKScroll(AKNode *parent = nullptr) noexcept;

    /** @brief The content's horizontal offset in logical pixels (<= 0 while scrolled right). */
    SkScalar offsetX() const noexcept;

    /** @brief The content's vertical offset in logical pixels (<= 0 while scrolled down). */
    SkScalar offsetY() const noexcept;

    /** @brief Sets both offsets at once, clamped to the content bounds. */
    void setOffset(SkScalar x, SkScalar y) noexcept;

    /** @brief Sets the horizontal offset, clamped to the content bounds. */
    void setOffsetX(Int32 x) noexcept;

    /** @brief Sets the vertical offset, clamped to the content bounds. */
    void setOffsetY(Int32 y) noexcept;

    /** @brief Sets the horizontal offset proportionally to the content width (input clamped to [0, 100]). */
    void setOffsetXPercent(SkScalar x) noexcept;

    /** @brief Sets the vertical offset proportionally to the content height (input clamped to [0, 100]). */
    void setOffsetYPercent(SkScalar y) noexcept;

    /** @brief The bottom-edge scroll bar. */
    AKScrollBar &horizontalBar() noexcept
    {
        return m_hBar;
    }

    /** @brief The right-edge scroll bar. */
    AKScrollBar &verticalBar() noexcept
    {
        return m_vBar;
    }

protected:
    void calculateContentBounds() noexcept;
    void applyConstraints() noexcept;

    /** @brief Pans the content; finger input drives the kinetic/elastic animations. */
    void pointerScrollEvent(const CZPointerScrollEvent &e) override;

    /** @brief Moves the arrow-key event filter from the old scene to the new one. */
    void sceneChangedEvent(const AKSceneChangedEvent &e) override;

    /** @brief Re-clamps the offset and updates the bars when the node is resized. */
    void layoutEvent(const CZLayoutEvent &e) override;

    /** @brief Scrolls by 128 logical pixels on arrow key presses delivered to the scene. */
    bool eventFilter(const CZEvent &event, CZObject &target) noexcept override;

    void updateBarXPrivate() noexcept;
    void updateBarYPrivate() noexcept;
    void moveXPrivate(SkScalar dx) noexcept;
    void moveYPrivate(SkScalar dy) noexcept;

    AKContainer m_slot;
    CZLinearAnimation m_kineticYAnim;
    CZLinearAnimation m_kineticXAnim;
    SkPoint m_vel { 0.f, 0.f };
    SkIRect m_contentBounds { 0, 0, 0, 0};
    Int64 m_lastFingerTimeX;
    Int64 m_lastFingerTimeY;
    AKScrollBar m_hBar { this, CZEdgeBottom, this };
    AKScrollBar m_vBar { this, CZEdgeRight, this };
    bool m_fingersDownX { false };
    bool m_fingersDownY { false };
};

#endif // CZ_AKSCROLL_H
