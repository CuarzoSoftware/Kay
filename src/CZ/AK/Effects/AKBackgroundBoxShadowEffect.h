#ifndef CZ_AKSHADOWEFFECT_H
#define CZ_AKSHADOWEFFECT_H

#include <CZ/AK/Effects/AKBackgroundEffect.h>
#include <CZ/AK/AKBorderRadius.h>
#include <CZ/skia/core/SkCanvas.h>

class CZ::AKBackgroundBoxShadowEffect : public AKBackgroundEffect
{
public:

    enum Changes
    {
        CHRadius = AKBackgroundEffect::CHLast,
        CHOffset,
        CHFillBackground,
        CHBorderRadius,
        CHLast
    };

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

    void setRadius(SkScalar radius) noexcept
    {
        if (m_radius == radius)
            return;

        addChange(CHRadius);
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

        addChange(CHOffset);
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

        addChange(CHFillBackground);
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

        addChange(CHBorderRadius);
        m_borderRadius = borderRadius;
    }

    const AKBorderRadius &borderRadius() const noexcept
    {
        return m_borderRadius;
    }

protected:
    using AKRenderable::enableReplaceImageColor;

    void onSceneCalculatedRect() override;
    void onSceneCalculatedRectWithBorderRadius() noexcept;
    void renderEvent(const AKRenderEvent &event) override;
    void onRenderWithBorderRadius(const AKRenderEvent &event) noexcept;
    void onTargetNodeChanged() override;

    std::shared_ptr<RSurface> m_surface;
    Int32 m_prevScale;
    SkIRect m_prevRect { -1, -1, 0, 0 };
    SkRect m_srcRects[9];
    SkIRect m_dstRects[9];
    SkIPoint m_offset;
    SkScalar m_radius;
    AKBorderRadius m_borderRadius { 0, 0, 0, 0};
    bool m_fillBackgroundEnabled;
};

#endif // CZ_AKSHADOWEFFECT_H
