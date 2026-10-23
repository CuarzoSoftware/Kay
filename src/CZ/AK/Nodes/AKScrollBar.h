#ifndef CZ_AKSCROLLBAR_H
#define CZ_AKSCROLLBAR_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKThreePatch.h>
#include <CZ/Core/CZLinearAnimation.h>
#include <CZ/Core/CZTimer.h>
#include <CZ/Core/CZEdge.h>
#include <CZ/Core/CZAdaptive.h>

/**
 * @brief Auto-hiding overlay scroll bar.
 * @ingroup AKNodes
 *
 * Used in pairs by AKScroll: a rail (the node itself, a three-patch image from
 * the theme) with a draggable rounded handle. The bar appears while the position
 * or size changes and fades out after one second of inactivity; hovering fades
 * it back in and widens the handle, and dragging (or clicking the rail) drives
 * the attached AKScroll's offset. A sizePercent() of 1 (content fully visible)
 * keeps it hidden.
 */
class CZ::AKScrollBar : public AKThreePatch
{
public:
    /** @brief Property change bits consumed on the next AKScene::render(). */
    enum Changes
    {
        /// edge() changed.
        CHEdge = AKThreePatch::CHLast,

        /// posPercent() changed.
        CHPosPercent,

        /// sizePercent() changed.
        CHSizePercent,

        CHLast ///< Marker for subclass chaining.
    };

    /** @brief Constructs a bar that controls the given AKScroll, anchored to the given edge. */
    AKScrollBar(AKScroll *scroll, CZEdge edge, AKNode *parent = nullptr) noexcept;

    /**
     * @brief Sets the edge of the scroll area the bar is anchored to.
     *
     * Left/right edges make it vertical, top/bottom horizontal (CZEdgeNone falls
     * back to CZEdgeRight). Reconfigures the layout, orientation and images.
     *
     * @return `true` if the edge changed, `false` otherwise.
     */
    bool setEdge(CZEdge edge) noexcept;

    /** @brief The edge the bar is anchored to. */
    CZEdge edge() const noexcept
    {
        return m_edge;
    }

    // [0,1] This does not affect AKScrol
    /**
     * @brief Sets the handle position along the rail, clamped to [0, 1].
     *
     * Purely visual: it does not scroll the attached AKScroll (which is the one
     * updating it). Shows the bar and restarts the fade-out timeout.
     *
     * @return `true` if the position changed, `false` otherwise.
     */
    bool setPosPercent(SkScalar pos) noexcept;

    /** @brief The handle position along the rail, in [0, 1]. */
    SkScalar posPercent() const noexcept { return m_posPercent; }

    // [0,1] This does not affect AKScrol
    /**
     * @brief Sets the handle length relative to the rail, clamped to [0, 1].
     *
     * Purely visual (see setPosPercent()). A value of 1 means the whole content
     * is visible and fades the bar out instead of showing it.
     *
     * @return `true` if the size changed, `false` otherwise.
     */
    bool setSizePercent(SkScalar size) noexcept;

    /** @brief The handle length relative to the rail, in [0, 1]. */
    SkScalar sizePercent() const noexcept { return m_sizePercent; }

    /** @brief Adaptive fill of the rail (track) behind the handle. Re-resolved on scheme changes. */
    void setRailBackgroundColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor railBackgroundColor() const noexcept { return m_railBackgroundColor; }

    /** @brief Adaptive color of the rail's inner border line. Re-resolved on scheme changes. */
    void setRailBorderColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor railBorderColor() const noexcept { return m_railBorderColor; }

    /** @brief Adaptive color of the draggable handle (tinted via replaceImageColor). */
    void setHandleColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor handleColor() const noexcept { return m_handleColor; }

protected:
    using AKThreePatch::setOrientation;
    using AKThreePatch::setImage;
    using AKThreePatch::setImageScale;
    void updatePosByPointer(SkScalar pointerPos) noexcept;

    /** @brief Starts/stops dragging on left press/release, grabbing the scene's pointer meanwhile. */
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;

    /** @brief While dragging, scrolls the attached AKScroll to the pointer position. */
    void pointerMoveEvent(const CZPointerMoveEvent &e) override;

    /** @brief Fades the bar in (widening the handle) and prevents it from hiding. */
    void pointerEnterEvent(const CZPointerEnterEvent &e) override;

    /** @brief Stops dragging, releases the pointer grab and schedules the fade-out. */
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;

    /** @brief Regenerates the rail/handle images when the scale or size changes. */
    void layoutEvent(const CZLayoutEvent &e) override;
    void updateImages() noexcept;
    void updateGeometry() noexcept;
    // Light values match the previous hardcoded ones; dark variants mirror them for a dark scheme.
    CZAdaptiveColor m_railBackgroundColor { 0xFFFBFBFB, 0xFF2E2E2E };
    CZAdaptiveColor m_railBorderColor { 0x19000000, 0x19FFFFFF };
    CZAdaptiveColor m_handleColor { 0x80000000, 0x80FFFFFF };
    SkScalar m_posPercent { 0.f };
    SkScalar m_sizePercent { 100.f };
    AKContainer m_space { YGFlexDirectionRow, false, this };
    AKThreePatch m_handle { CZOrientation::H, this };
    CZEdge m_edge { CZEdgeNone };
    CZTimer m_fadeOutTimer;
    CZLinearAnimation m_fadeOutAnim;
    CZLinearAnimation m_hoverAnim;
    CZWeak<AKScroll> m_scroll;
    bool m_dragging { false };
    bool m_preventHide { false };
};

#endif // CZ_AKSCROLLBAR_H
