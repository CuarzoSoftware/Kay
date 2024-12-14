#ifndef AKRENDERABLE_H
#define AKRENDERABLE_H

#include <AK/nodes/AKNode.h>
#include <AK/AKPainter.h>

class AK::AKRenderable : public AKNode
{
public:
    AKRenderable(AKNode *parent = nullptr) noexcept : AKNode(parent) { m_caps |= Render; }
    void addDamage(const SkRegion &region) noexcept;
    void addDamage(const SkIRect &rect) noexcept;
    const SkRegion &damage() const noexcept;
    SkRegion opaqueRegion;

    enum class ColorHint
    {
        /// Fully paque (default)
        Opaque,

        /// Fully translucent
        Translucent,

        /// User defined region (see opaqueRegion)
        UseOpaqueRegion
    };

    enum Changes
    {
        Chg_Opacity = AKNode::Chg_Last,
        Chg_CustomTextureColorEnabled,
        Chg_Color,
        Chg_ColorFactor,
        Chg_ColorHint,
        Chg_CustomBlendFuncEnabled,
        Chg_CustomBlendFunc,

        Chg_Last
    };

    void setColorHint(ColorHint hint) noexcept
    {
        if (m_colorHint == hint)
            return;

        addChange(Chg_ColorHint);
        m_colorHint = hint;
    }

    ColorHint colorHint() const noexcept
    {
        return m_colorHint;
    }

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

    void setColor(const SkColor4f &color) noexcept
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

protected:
    friend class AKScene;
    friend class AKSubScene;
    SkColor4f m_color { 1.f, 1.f, 1.f, 1.f }; // fA used for opacity
    SkColor4f m_colorFactor { 1.f, 1.f, 1.f, 1.f };
    AKBlendFunc m_customBlendFunc { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA };
    ColorHint m_colorHint;
    bool m_customBlendFuncEnabled { false };
    bool m_customTextureColorEnabled { false };
    virtual void onRender(AKPainter *painter, const SkRegion &damage) = 0;
};

#endif // AKRENDERABLE_H
