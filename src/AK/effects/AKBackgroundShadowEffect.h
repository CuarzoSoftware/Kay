#ifndef AKSHADOWEFFECT_H
#define AKSHADOWEFFECT_H

#include "AK/AKBrush.h"
#include <AK/effects/AKBackgroundEffect.h>
#include <bitset>
#include <include/core/SkCanvas.h>

class AK::AKBackgroundShadowEffect : public AKBackgroundEffect
{
public:

    enum ShadowType
    {
        /* A fast, pre baked shadow with optional border radius. */
        Box
    };

    enum Changes
    {
        Chg_ShadowType = AKBackgroundEffect::Chg_Last,
        Chg_ShadowRadius,
        Chg_ShadowOffset,
        Chg_ShadowClippingEnabled,

        Chg_Last
    };

    explicit AKBackgroundShadowEffect(ShadowType type, SkScalar radius,
                             const SkIPoint &offset, SkColor color,
                             bool clipping, AKNode *targetNode = nullptr) noexcept :
        AKBackgroundEffect(Background)
    {
        setColorHint(ColorHint::Translucent);
        enableCustomTextureColor(true);
        setShadowType(type);
        setShadowRadius(radius);
        setShadowOffset(offset);
        setColor(SkColor4f::FromColor(color));
        enableShadowClipping(clipping);

        if (targetNode)
            targetNode->setBackgroundEffect(this);
    };

    void setShadowType(ShadowType type) noexcept
    {
        if (m_shadowType == type)
            return;

        addChange(Chg_ShadowType);
        m_shadowType = type;
    }

    ShadowType shadowType() const noexcept
    {
        return m_shadowType;
    }

    void setShadowRadius(SkScalar radius) noexcept
    {
        if (m_shadowRadius == radius)
            return;

        addChange(Chg_ShadowRadius);
        m_shadowRadius = radius;
    }

    SkScalar shadowRadius() const noexcept
    {
        return m_shadowRadius;
    }

    void setShadowOffset(const SkIPoint &offset) noexcept
    {
        if (m_shadowOffset == offset)
            return;

        addChange(Chg_ShadowOffset);
        m_shadowOffset = offset;
    }

    void setShadowOffset(Int32 x, Int32 y) noexcept
    {
        setShadowOffset({x, y});
    }

    const SkIPoint &offset() const noexcept
    {
        return m_shadowOffset;
    }

    void enableShadowClipping(bool enabled) noexcept
    {
        if (m_shadowClippingEnabled == enabled)
            return;

        addChange(Chg_ShadowClippingEnabled);
        m_shadowClippingEnabled = enabled;
    }

    bool shadowClippingEnabled() const noexcept
    {
        return m_shadowClippingEnabled;
    }


protected:
    using AKRenderable::enableCustomTextureColor;
    using AKRenderable::setColorHint;

    void onSceneBegin() override;
    void onSceneBeginBox() noexcept;

    void onRender(AKPainter *painter, const SkRegion &damage) override;
    void onRenderBox(AKPainter *painter, const SkRegion &damage) noexcept;

    void onTargetNodeChanged() override;
    struct ShadowData
    {
        std::shared_ptr<AKSurface> surface;
        SkVector prevScale;
        SkRect srcRects[9];
        SkIRect dstRects[9];
    };

    std::unordered_map<AKTarget*, ShadowData> m_targets;
    ShadowData *m_currentData;
    SkIPoint m_shadowOffset;
    SkScalar m_shadowRadius;
    ShadowType m_shadowType;
    bool m_shadowClippingEnabled;
};

#endif // AKSHADOWEFFECT_H
