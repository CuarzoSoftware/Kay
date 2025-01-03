#ifndef AKSHADOWEFFECT_H
#define AKSHADOWEFFECT_H

#include <AK/effects/AKBackgroundEffect.h>
#include <AK/AKBorderRadius.h>
#include <include/core/SkCanvas.h>

class AK::AKBackgroundBoxShadowEffect : public AKBackgroundEffect
{
public:

    enum Changes
    {
        Chg_Radius = AKBackgroundEffect::Chg_Last,
        Chg_Offset,
        Chg_FillBackground,
        Chg_BorderRadius,
        Chg_Last
    };

    explicit AKBackgroundBoxShadowEffect(SkScalar radius,
                             const SkIPoint &offset, SkColor color,
                             bool fillBackground, AKNode *targetNode = nullptr) noexcept :
        AKBackgroundEffect(Background),
        m_offset(offset),
        m_radius(radius),
        m_fillBackgroundEnabled(fillBackground)
    {
        enableCustomTextureColor(true);
        setColorWithAlpha(color);

        if (targetNode)
            targetNode->addBackgroundEffect(this);
    }

    void setRadius(SkScalar radius) noexcept
    {
        if (m_radius == radius)
            return;

        addChange(Chg_Radius);
        m_radius = radius;
    }

    SkScalar radius() const noexcept
    {
        return m_radius;
    }

    void setOffset(const SkIPoint &offset) noexcept
    {
        if (m_offset == offset)
            return;

        addChange(Chg_Offset);
        m_offset = offset;
    }

    void setOffset(Int32 x, Int32 y) noexcept
    {
        setOffset({x, y});
    }

    const SkIPoint &offset() const noexcept
    {
        return m_offset;
    }

    void enableFillBackground(bool enabled) noexcept
    {
        if (m_fillBackgroundEnabled == enabled)
            return;

        addChange(Chg_FillBackground);
        m_fillBackgroundEnabled = enabled;
    }

    bool fillBackgroundEnabled() const noexcept
    {
        return m_fillBackgroundEnabled;
    }

    void setBorderRadius(const AKBorderRadius &borderRadius) noexcept
    {
        if (m_borderRadius == borderRadius)
            return;

        addChange(Chg_BorderRadius);
        m_borderRadius = borderRadius;
    }

    const AKBorderRadius &borderRadius() const noexcept
    {
        return m_borderRadius;
    }

protected:
    using AKRenderable::enableCustomTextureColor;

    void onLayoutUpdate() override;
    void onLayoutUpdateWithBorderRadius() noexcept;
    void onRender(AKPainter *painter, const SkRegion &damage) override;
    void onRenderWithBorderRadius(AKPainter *painter, const SkRegion &damage) noexcept;
    void onTargetNodeChanged() override;
    struct ShadowData
    {
        std::shared_ptr<AKSurface> surface;
        Int32 prevScale;
        SkRect srcRects[9];
        SkIRect dstRects[9];
    };

    std::unordered_map<AKTarget*, ShadowData> m_targets;
    ShadowData *m_currentData;
    SkIPoint m_offset;
    SkScalar m_radius;
    AKBorderRadius m_borderRadius { 0, 0, 0, 0};
    bool m_fillBackgroundEnabled;
};

#endif // AKSHADOWEFFECT_H
