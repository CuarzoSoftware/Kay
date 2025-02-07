#ifndef AKRENDERABLE_H
#define AKRENDERABLE_H

#include <AK/nodes/AKNode.h>
#include <AK/AKPainter.h>

class AK::AKRenderable : public AKNode
{
public:
    enum RenderableHint
    {
        SolidColor,
        Texture
    };

    enum Changes
    {
        Chg_Opacity = AKNode::Chg_Last,
        Chg_CustomTextureColorEnabled,
        Chg_Color,
        Chg_ColorFactor,
        Chg_CustomBlendFuncEnabled,
        Chg_CustomBlendFunc,
        Chg_Size,
        Chg_DiminishOpacityOnInactive,
        Chg_Last
    };

    AKRenderable(RenderableHint hint, AKNode *parent = nullptr) noexcept :
        AKNode(parent),
        m_renderableHint(hint)
    { m_caps |= Render; }

    void addDamage(const SkRegion &region) noexcept;
    void addDamage(const SkIRect &rect) noexcept;
    const SkRegion &damage() const noexcept;
    SkRegion opaqueRegion;

    void setOpacity(SkScalar alpha) noexcept
    {
        if (m_color.fA == alpha)
            return;

        m_color.fA = alpha;
        addChange(Chg_Opacity);
    }

    SkScalar opacity() const noexcept
    {
        return m_color.fA;
    }

    void enableCustomTextureColor(bool enable) noexcept
    {
        if (m_customBlendFuncEnabled == enable)
            return;

        m_customTextureColorEnabled = enable;
        addChange(Chg_CustomTextureColorEnabled);
    }

    bool customTextureColorEnabled() const noexcept
    {
        return m_customTextureColorEnabled;
    }

    void setColorWithAlpha(const SkColor4f &color) noexcept
    {
        if (color.fR != m_color.fR ||
            color.fG != m_color.fG ||
            color.fB != m_color.fB)
        {
            m_color.fR = color.fR;
            m_color.fG = color.fG;
            m_color.fB = color.fB;
            addChange(Chg_Color);
        }

        if (color.fA != m_color.fA)
        {
            m_color.fA = color.fA;
            addChange(Chg_Opacity);
        }
    }

    void setColorWithAlpha(SkColor color) noexcept
    {
        setColorWithAlpha(SkColor4f::FromColor(color));
    }

    void setColorWithoutAlpha(const SkColor4f &color) noexcept
    {
        if (color.fR == m_color.fR &&
            color.fG == m_color.fG &&
            color.fB == m_color.fB)
            return;

        m_color.fR = color.fR;
        m_color.fG = color.fG;
        m_color.fB = color.fB;
        addChange(Chg_Color);
    }

    void setColorWithoutAlpha(SkColor color) noexcept
    {
        setColorWithoutAlpha(SkColor4f::FromColor(color));
    }

    const SkColor4f &color() const noexcept
    {
        return m_color;
    }

    void setColorFactor(const SkColor4f &colorFactor) noexcept
    {
        if (m_colorFactor == colorFactor)
            return;

        m_colorFactor = colorFactor;
        addChange(Chg_ColorFactor);
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
        if (m_customBlendFuncEnabled == enable)
            return;

        m_customBlendFuncEnabled = enable;
        addChange(Chg_CustomBlendFuncEnabled);
    }

    bool customBlendFuncEnabled() const noexcept
    {
        return m_customBlendFuncEnabled;
    }

    void setCustomBlendFunc(const AKBlendFunc &blendFunc) noexcept
    {
        if (m_customBlendFunc == blendFunc)
            return;

        m_customBlendFunc = blendFunc;
        addChange(Chg_CustomBlendFunc);
    }

    const AKBlendFunc &customBlendFunc() const noexcept
    {
        return m_customBlendFunc;
    }

    void enableDiminishOpacityOnInactive(bool enable) noexcept
    {
        if (m_flags.check(DiminishOpacityOnInactive) == enable)
            return;

        m_flags.setFlag(DiminishOpacityOnInactive, enable);
        addChange(Chg_DiminishOpacityOnInactive);
    }

    bool diminishOpacityOnInactive() const noexcept
    {
        return m_flags.check(DiminishOpacityOnInactive);
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
    SkColor4f m_color { SkColors::kBlack }; // fA used for opacity
    SkColor4f m_colorFactor { 1.f, 1.f, 1.f, 1.f };
    AKBlendFunc m_customBlendFunc { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA };
    RenderableHint m_renderableHint;
    ColorHint m_colorHint { UseRegion };
    bool m_customBlendFuncEnabled { false };
    bool m_customTextureColorEnabled { false };
    virtual void onEvent(const AKEvent &event) override;
    virtual void onRender(AKPainter *painter, const SkRegion &damage, const SkIRect &rect) = 0;
private:
    void handleCommonChanges() noexcept;
};

#endif // AKRENDERABLE_H
