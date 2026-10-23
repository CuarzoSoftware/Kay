#ifndef CZ_AKRENDERABLE_H
#define CZ_AKRENDERABLE_H

#include <CZ/AK/Nodes/AKNode.h>
#include <CZ/Ream/RBlendMode.h>

/**
 * @brief A node that renders directly into an AKTarget.
 *
 * Renderables draw their content during renderEvent() using the Ream painter
 * (see AKRenderEvent), which uses efficient internal shaders — ideal for images
 * (e.g. textures from Wayland surfaces), solid fills and masks. For complex Skia
 * drawing prefer AKBakeable. See @ref creating_components_page "Creating Components".
 *
 * @ingroup AKNodes
 */
class CZ::AKRenderable : public AKNode
{
public:
    /**
     * @brief Hint about the nature of the content, used by the scene for
     *        opacity/color-related optimizations.
     */
    enum class RenderableHint
    {
        SolidColor, ///< The node is filled with color() (e.g. AKSolidColor).
        Image       ///< The node displays image content.
    };

    /**
     * @brief Changes tracked in addition to AKNode::Changes.
     */
    enum Changes
    {
        CHOpacity = AKNode::CHLast,   ///< opacity() changed.
        CHReplaceImageColorEnabled,   ///< replaceImageColorEnabled() changed.
        CHColor,                      ///< color() changed.
        CHColorFactor,                ///< colorFactor() changed.
        CHCustomBlendModeEnabled,     ///< customBlendModeEnabled() changed.
        CHCustomBlendMode,            ///< customBlendMode() changed.
        CHSize,                       ///< The content size changed.
        CHDiminishOpacityOnInactive,  ///< diminishOpacityOnInactive() changed.
        CHLast                        ///< Marker for subclass chaining.
    };

    /**
     * @brief Constructor.
     *
     * @param hint Content hint, see RenderableHint.
     * @param parent Optional parent node.
     */
    AKRenderable(RenderableHint hint, AKNode *parent = nullptr) noexcept;

    /**
     * @brief Adds damage (a region to repaint) in node-local coordinates.
     *
     * The damage is recorded for every target the node is presented on and consumed
     * during the next AKScene::render(). Layout-driven damage (moving/resizing) is
     * handled by the scene — call this only when the content itself changes.
     */
    void addDamage(const SkRegion &region) noexcept;

    /** @copydoc addDamage(const SkRegion&) */
    void addDamage(const SkIRect &rect) noexcept;

    /**
     * @brief Accumulated damage for the target currently being rendered.
     */
    const SkRegion &damage() const noexcept;

    /**
     * @brief Region (in node-local coordinates) guaranteed to be fully opaque.
     *
     * Used by the scene for occlusion culling and to skip blending. Keep it up
     * to date when the content or size changes (typically in onSceneBegin()).
     */
    SkRegion opaqueRegion;

    /**
     * @brief Region (in node-local coordinates) that produces no output at all,
     *        letting the scene skip rendering it entirely.
     */
    SkRegion invisibleRegion;

    /**
     * @brief Sets the node opacity in [0, 1], applied by the scene when compositing. Default is 1.
     */
    void setOpacity(SkScalar opacity) noexcept
    {
        opacity = std::clamp(opacity, 0.f, 1.f);
        if (m_opacity == opacity)
            return;

        m_opacity = opacity;
        addChange(CHOpacity);
    }

    /**
     * @brief The node opacity (see setOpacity()).
     */
    SkScalar opacity() const noexcept
    {
        return m_opacity;
    }

    /**
     * @brief When enabled, the image's RGB is replaced with color() while keeping the
     *        image's alpha — useful to tint masks/icons. Disabled by default.
     */
    void enableReplaceImageColor(bool enable) noexcept
    {
        if (m_replaceImageColorEnabled == enable)
            return;

        m_replaceImageColorEnabled = enable;
        addChange(CHReplaceImageColorEnabled);
    }

    /**
     * @brief Whether image color replacement is enabled (see enableReplaceImageColor()).
     */
    bool replaceImageColorEnabled() const noexcept
    {
        return m_replaceImageColorEnabled;
    }

    /**
     * @brief Sets the solid color (unpremultiplied alpha).
     *
     * Used as the fill for RenderableHint::SolidColor nodes, or as the replacement
     * color when replaceImageColorEnabled() is set. Default is black.
     */
    void setColor(SkColor color) noexcept
    {
        if (color == m_color)
            return;

        m_color = color;
        addChange(CHColor);
    }

    /**
     * @brief The solid color (see setColor()).
     */
    SkColor color() const noexcept
    {
        return m_color;
    }

    /**
     * @brief The solid color as SkColor4f (see setColor()).
     */
    SkColor4f color4f() const noexcept
    {
        return SkColor4f::FromColor(m_color);
    }

    /**
     * @brief Sets a factor each rendered color channel is multiplied by. Default is (1, 1, 1, 1).
     */
    void setColorFactor(const SkColor4f &colorFactor) noexcept
    {
        if (m_colorFactor == colorFactor)
            return;

        m_colorFactor = colorFactor;
        addChange(CHColorFactor);
    }

    /** @copydoc setColorFactor(const SkColor4f&) */
    void setColorFactor(SkColor colorFactor) noexcept
    {
        setColorFactor(SkColor4f::FromColor(colorFactor));
    }

    /**
     * @brief The color factor (see setColorFactor()).
     */
    const SkColor4f &colorFactor() const noexcept
    {
        return m_colorFactor;
    }

    /**
     * @brief Toggles the use of customBlendMode() instead of the default SrcOver blending.
     *        Disabled by default.
     */
    void enableCustomBlendFunc(bool enable) noexcept
    {
        if (m_customBlendModeEnabled == enable)
            return;

        m_customBlendModeEnabled = enable;
        addChange(CHCustomBlendModeEnabled);
    }

    /**
     * @brief Whether the custom blend mode is used (see enableCustomBlendFunc()).
     */
    bool customBlendModeEnabled() const noexcept
    {
        return m_customBlendModeEnabled;
    }

    /**
     * @brief Sets the blend mode used while customBlendModeEnabled() is set. Default is SrcOver.
     */
    void setCustomBlendMode(RBlendMode blendMode) noexcept
    {
        if (m_customBlendMode == blendMode)
            return;

        m_customBlendMode = blendMode;
        addChange(CHCustomBlendMode);
    }

    /**
     * @brief The custom blend mode (see setCustomBlendMode()).
     */
    RBlendMode customBlendMode() const noexcept
    {
        return m_customBlendMode;
    }

    /**
     * @brief When enabled, the node is rendered with reduced opacity while the parent
     *        scene's window is deactivated (like macOS inactive window controls).
     *        Disabled by default.
     */
    void enableDiminishOpacityOnInactive(bool enable) noexcept
    {
        if (m_flags.has(DiminishOpacityOnInactive) == enable)
            return;

        m_flags.setFlag(DiminishOpacityOnInactive, enable);
        addChange(CHDiminishOpacityOnInactive);
    }

    /**
     * @brief Whether opacity is diminished while inactive (see enableDiminishOpacityOnInactive()).
     */
    bool diminishOpacityOnInactive() const noexcept
    {
        return m_flags.has(DiminishOpacityOnInactive);
    }

protected:
    enum ColorHint
    {
        UseRegion,
        Opaque,
        Translucent
    };
    friend class AKScene;
    friend class AKSubScene;
    SkScalar m_opacity { 1.f };
    SkColor m_color { SK_ColorBLACK };
    SkColor4f m_colorFactor { 1.f, 1.f, 1.f, 1.f };
    RBlendMode m_customBlendMode { RBlendMode::SrcOver };
    RenderableHint m_renderableHint;
    ColorHint m_colorHint { UseRegion };
    bool m_customBlendModeEnabled { false };
    bool m_replaceImageColorEnabled { false };
    bool event(const CZEvent &event) noexcept override;
    void windowStateEvent(const CZWindowStateEvent &event) override;

    /**
     * @brief Render event, triggered during AKScene::render() for each damaged region set.
     *
     * Draw with the Ream painter (AKRenderEvent::pass), restricted to AKRenderEvent::damage.
     * Coordinates are relative to the current target surface (see AKRenderEvent::rect).
     */
    virtual void renderEvent(const AKRenderEvent &event) = 0;
private:
    void handleCommonChanges() noexcept;
};

#endif // CZ_AKRENDERABLE_H
