#ifndef CZ_AKBACKGROUNDPATHSHADOWEFFECT_H
#define CZ_AKBACKGROUNDPATHSHADOWEFFECT_H

#include <CZ/AK/Effects/AKBackgroundEffect.h>
#include <CZ/skia/core/SkCanvas.h>

// Only works with AKBackeables
class CZ::AKBackgroundImageShadowEffect : public AKBackgroundEffect
{
public:

    enum Changes
    {
        CHRadius = AKBackgroundEffect::CHLast,
        CHOffset,
        CHLast
    };

    explicit AKBackgroundImageShadowEffect(SkScalar radius,
                                         const SkIPoint &offset, SkColor color,
                                        AKNode *targetNode = nullptr) noexcept :
        AKBackgroundEffect(Background),
        m_offset(offset),
        m_radius(radius)
    {
        enableCustomTextureColor(true);
        setColorWithAlpha(color);

        if (targetNode)
            targetNode->addBackgroundEffect(this);
    }

    CZ_DISABLE_COPY(AKBackgroundImageShadowEffect)

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

protected:
    using AKRenderable::enableCustomTextureColor;

    void onSceneCalculatedRect() override;
    void renderEvent(const AKRenderEvent &event) override;
    void onTargetNodeChanged() override;

    std::shared_ptr<RSurface> m_surface;
    Int32 m_prevScale;
    SkIPoint m_offset;
    SkScalar m_radius;
};

#endif // CZ_AKBACKGROUNDPATHSHADOWEFFECT_H
