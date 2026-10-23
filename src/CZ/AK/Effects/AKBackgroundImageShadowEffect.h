#ifndef CZ_AKBACKGROUNDPATHSHADOWEFFECT_H
#define CZ_AKBACKGROUNDPATHSHADOWEFFECT_H

#include <CZ/AK/Effects/AKBackgroundEffect.h>
#include <CZ/skia/core/SkCanvas.h>

// Only works with AKBackeables
/**
 * @brief Drop shadow shaped after the target node's baked image.
 *
 * Renders a Gaussian-blurred silhouette of the target node's baked surface behind it,
 * so the shadow follows the actual (possibly non-rectangular) content shape instead of
 * a plain box. The shadow is baked into an intermediate surface with Skia's
 * DropShadowOnly image filter and tinted with color() (replace-image-color is enabled).
 *
 * Attach it with AKNode::addBackgroundEffect().
 *
 * @note Only works with AKBakeable targets; it does nothing if the target is not an
 *       AKBakeable or has no baked surface yet.
 *
 * @ingroup AKEffects
 */
class CZ::AKBackgroundImageShadowEffect : public AKBackgroundEffect
{
public:

    /**
     * @brief Flags for tracking property changes.
     */
    enum Changes
    {
        CHRadius = AKBackgroundEffect::CHLast, ///< The blur radius changed.
        CHOffset,                              ///< The shadow offset changed.
        CHLast                                 ///< Marker for subclass chaining.
    };

    /**
     * @brief Constructs an image shadow effect.
     *
     * @param radius Blur radius in logical pixels (the shadow extends this far beyond the node).
     * @param offset Shadow offset relative to the target node, in logical pixels.
     * @param color Shadow tint color.
     * @param targetNode If not `nullptr`, the effect is immediately attached to it
     *        via AKNode::addBackgroundEffect().
     */
    explicit AKBackgroundImageShadowEffect(SkScalar radius,
                                           const SkIPoint &offset, SkColor color,
                                           AKBakeable *targetNode = nullptr) noexcept;

    /** @brief Sets the blur radius in logical pixels. */
    void setRadius(SkScalar radius) noexcept
    {
        if (m_radius == radius)
            return;

        addChange(CHRadius);
        m_radius = radius;
    }

    /** @brief Returns the blur radius in logical pixels. */
    SkScalar radius() const noexcept
    {
        return m_radius;
    }

    /** @brief Sets the shadow offset relative to the target node, in logical pixels. */
    void setOffset(const SkIPoint &offset) noexcept
    {
        if (m_offset == offset)
            return;

        addChange(CHOffset);
        m_offset = offset;
    }

    /** @brief Sets the shadow offset relative to the target node, in logical pixels. */
    void setOffset(Int32 x, Int32 y) noexcept
    {
        setOffset({x, y});
    }

    /** @brief Returns the shadow offset relative to the target node. */
    const SkIPoint &offset() const noexcept
    {
        return m_offset;
    }

protected:
    using AKRenderable::enableReplaceImageColor;

    /** @brief Recalculates effectRect and re-bakes the shadow when the target's size, scale or bake damage changes. */
    void targetNodeRectCalculated() override;
    /** @brief Draws the baked shadow image behind the target node. */
    void renderEvent(const AKRenderEvent &event) override;
    /** @brief Notified when the effect is attached to a different target node (no-op). */
    void onTargetNodeChanged() override;

    std::shared_ptr<RSurface> m_surface;
    Int32 m_prevScale;
    SkIPoint m_offset;
    SkScalar m_radius;
};

#endif // CZ_AKBACKGROUNDPATHSHADOWEFFECT_H
