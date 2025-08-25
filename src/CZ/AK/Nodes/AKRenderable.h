#ifndef CZ_AKRENDERABLE_H
#define CZ_AKRENDERABLE_H

#include <CZ/AK/Nodes/AKNode.h>
#include <CZ/Ream/RBlendMode.h>

/**
 * @brief A node that renders into an AKTarget.
 * @ingroup AKNodes
 */
class CZ::AKRenderable : public AKNode
{
public:
    enum class RenderableHint
    {
        SolidColor,
        Image
    };

    enum Changes
    {
        CHOpacity = AKNode::CHLast,
        CHReplaceImageColorEnabled,
        CHColor,
        CHColorFactor,
        CHCustomBlendModeEnabled,
        CHCustomBlendMode,
        CHSize,
        CHDiminishOpacityOnInactive,
        CHLast
    };

    AKRenderable(RenderableHint hint, AKNode *parent = nullptr) noexcept :
        AKNode(parent),
        m_renderableHint(hint) {
        m_caps |= RenderableBit;
    }

    void addDamage(const SkRegion &region) noexcept;
    void addDamage(const SkIRect &rect) noexcept;
    const SkRegion &damage() const noexcept;
    SkRegion opaqueRegion;
    SkRegion invisibleRegion;

    void setOpacity(SkScalar opacity) noexcept
    {
        opacity = std::clamp(opacity, 0.f, 1.f);
        if (m_opacity == opacity)
            return;

        m_opacity = opacity;
        addChange(CHOpacity);
    }

    SkScalar opacity() const noexcept
    {
        return m_opacity;
    }

    void enableReplaceImageColor(bool enable) noexcept
    {
        if (m_replaceImageColorEnabled == enable)
            return;

        m_replaceImageColorEnabled = enable;
        addChange(CHReplaceImageColorEnabled);
    }

    bool replaceImageColorEnabled() const noexcept
    {
        return m_replaceImageColorEnabled;
    }

    // Unpremult alpha
    void setColor(SkColor color) noexcept
    {
        if (color == m_color)
            return;

        m_color = color;
        addChange(CHColor);
    }

    SkColor color() const noexcept
    {
        return m_color;
    }

    SkColor4f color4f() const noexcept
    {
        return SkColor4f::FromColor(m_color);
    }

    void setColorFactor(const SkColor4f &colorFactor) noexcept
    {
        if (m_colorFactor == colorFactor)
            return;

        m_colorFactor = colorFactor;
        addChange(CHColorFactor);
    }

    void setColorFactor(SkColor colorFactor) noexcept
    {
        setColorFactor(SkColor4f::FromColor(colorFactor));
    }

    const SkColor4f &colorFactor() const noexcept
    {
        return m_colorFactor;
    }

    void enableCustomBlendFunc(bool enable) noexcept
    {
        if (m_customBlendModeEnabled == enable)
            return;

        m_customBlendModeEnabled = enable;
        addChange(CHCustomBlendModeEnabled);
    }

    bool customBlendModeEnabled() const noexcept
    {
        return m_customBlendModeEnabled;
    }

    void setCustomBlendMode(RBlendMode blendMode) noexcept
    {
        if (m_customBlendMode == blendMode)
            return;

        m_customBlendMode = blendMode;
        addChange(CHCustomBlendMode);
    }

    RBlendMode customBlendMode() const noexcept
    {
        return m_customBlendMode;
    }

    void enableDiminishOpacityOnInactive(bool enable) noexcept
    {
        if (m_flags.has(DiminishOpacityOnInactive) == enable)
            return;

        m_flags.setFlag(DiminishOpacityOnInactive, enable);
        addChange(CHDiminishOpacityOnInactive);
    }

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
    virtual void renderEvent(const AKRenderEvent &event) = 0;
private:
    void handleCommonChanges() noexcept;
};

#endif // CZ_AKRENDERABLE_H
