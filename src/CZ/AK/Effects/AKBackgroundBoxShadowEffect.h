#ifndef CZ_AKSHADOWEFFECT_H
#define CZ_AKSHADOWEFFECT_H

#include <CZ/AK/Effects/AKBackgroundEffect.h>
#include <CZ/Core/CZBorderRadius.h>
#include <CZ/skia/core/SkCanvas.h>

/**
 * @brief Box-shaped drop shadow rendered behind a target node.
 *
 * Renders a Gaussian-blurred rectangle (optionally with rounded corners) behind the node
 * it is attached to via AKNode::addBackgroundEffect(). The shadow is baked once into a
 * small white mask surface and reused: without border radius it is drawn as nine stretched
 * patches, with border radius as a single full-size image. The mask is tinted with color()
 * (replace-image-color is enabled).
 *
 * When fill background is disabled, the area covered by the target node is excluded from
 * rendering, so only the visible shadow ring is drawn (useful for translucent nodes).
 *
 * @ingroup AKEffects
 */
class CZ::AKBackgroundBoxShadowEffect : public AKBackgroundEffect
{
public:

    /**
     * @brief Flags for tracking property changes.
     */
    enum Changes
    {
        CHRadius = AKBackgroundEffect::CHLast, ///< The blur radius changed.
        CHOffset,                              ///< The shadow offset changed.
        CHFillBackground,                      ///< The fill background mode changed.
        CHBorderRadius,                        ///< The border radius changed.
        CHLast                                 ///< Marker for subclass chaining.
    };

    /**
     * @brief Constructs a box shadow effect.
     *
     * @param radius Blur radius in logical pixels (the shadow extends this far beyond the node).
     * @param offset Shadow offset relative to the target node, in logical pixels.
     * @param color Shadow tint color.
     * @param fillBackground If `true`, the shadow is also drawn under the target node;
     *        if `false`, the node's area is excluded.
     * @param targetNode If not `nullptr`, the effect is immediately attached to it
     *        via AKNode::addBackgroundEffect().
     */
    explicit AKBackgroundBoxShadowEffect(SkScalar radius,
                             const SkIPoint &offset, SkColor color,
                             bool fillBackground, AKNode *targetNode = nullptr) noexcept :
        AKBackgroundEffect(Background),
        m_offset(offset),
        m_radius(radius),
        m_fillBackgroundEnabled(fillBackground)
    {
        enableReplaceImageColor(true);
        setColor(color);

        if (targetNode)
            targetNode->addBackgroundEffect(this);
    }

    CZ_DISABLE_COPY(AKBackgroundBoxShadowEffect)

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

    /**
     * @brief Toggles drawing the shadow under the target node.
     *
     * When disabled, the target node's area is excluded from rendering,
     * leaving only the visible shadow ring (for translucent nodes).
     */
    void enableFillBackground(bool enabled) noexcept
    {
        if (m_fillBackgroundEnabled == enabled)
            return;

        addChange(CHFillBackground);
        m_fillBackgroundEnabled = enabled;
    }

    /** @brief Indicates whether the shadow is also drawn under the target node. */
    bool fillBackgroundEnabled() const noexcept
    {
        return m_fillBackgroundEnabled;
    }

    /**
     * @brief Sets the target node's border radius so the shadow matches its rounded corners.
     *
     * Defaults to 0 for all corners (plain rectangle).
     */
    void setBorderRadius(const CZBorderRadius &borderRadius) noexcept
    {
        if (m_borderRadius == borderRadius)
            return;

        addChange(CHBorderRadius);
        m_borderRadius = borderRadius;
    }

    /** @brief Returns the border radius used to shape the shadow. */
    const CZBorderRadius &borderRadius() const noexcept
    {
        return m_borderRadius;
    }

protected:
    using AKRenderable::enableReplaceImageColor;

    /** @brief Recalculates effectRect and (re)bakes the shadow mask when the target rect, scale or properties change. */
    void targetNodeRectCalculated() override;
    /** @brief Variant of targetNodeRectCalculated() used when any border radius corner is non-zero. */
    void onSceneCalculatedRectWithBorderRadius() noexcept;
    /** @brief Draws the baked shadow as nine stretched patches (no border radius path). */
    void renderEvent(const AKRenderEvent &event) override;
    /** @brief Draws the baked shadow as a single image (border radius path). */
    void onRenderWithBorderRadius(const AKRenderEvent &event) noexcept;
    /** @brief Notified when the effect is attached to a different target node (no-op). */
    void onTargetNodeChanged() override;

    std::shared_ptr<RSurface> m_surface;
    Int32 m_prevScale;
    SkIRect m_prevRect { -1, -1, 0, 0 };
    SkRect m_srcRects[9];
    SkIRect m_dstRects[9];
    SkIPoint m_offset;
    SkScalar m_radius;
    CZBorderRadius m_borderRadius { 0, 0, 0, 0};
    bool m_fillBackgroundEnabled;
};

#endif // CZ_AKSHADOWEFFECT_H
