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

    explicit AKBackgroundShadowEffect(ShadowType type, SkScalar radius,
                             const SkIPoint &offset, SkColor color,
                             bool clipping, AKNode *targetNode = nullptr) noexcept :
        AKBackgroundEffect(Background),
        m_offset(offset),
        m_radius(radius),
        m_color(color),
        m_type(type)
    {
        enableClipping(clipping);
        if (targetNode)
            targetNode->setBackgroundEffect(this);
    };

    void setType(ShadowType type) noexcept
    {
        m_type = type;
    }

    ShadowType type() const noexcept
    {
        return m_type;
    }

    void setRadius(SkScalar radius) noexcept
    {
        m_radius = radius;
    }

    SkScalar radius() const noexcept
    {
        return m_radius;
    }

    void setOffset(const SkIPoint &offset) noexcept
    {
        m_offset = offset;
    }

    void setOffset(Int32 x, Int32 y) noexcept
    {
        m_offset = {x, y};
    }

    const SkIPoint &offset() const noexcept
    {
        return m_offset;
    }

    void setColor(SkColor color) noexcept
    {
        m_color = color;
    }

    SkColor color() const noexcept
    {
        return m_color;
    }

    void enableClipping(bool enabled) noexcept
    {
        m_flags.set(Clipping, enabled);
    }

    bool clippingEnabled() const noexcept
    {
        return m_flags.test(Clipping);
    }

private:
    enum Flags
    {
        Clipping,
        FlagsLast
    };
    void onSceneBegin() override;
    void onSceneBeginBox() noexcept;

    void onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque) override;
    void onRenderBox(SkCanvas *canvas, const SkRegion &damage) noexcept;

    void onTargetNodeChanged() override;
    struct ShadowData
    {
        std::shared_ptr<AKSurface> shadowSurface;
        SkIRect nineCenter;
        SkScalar radius;
        AKBrush brush;
        std::bitset<FlagsLast> flags;
    };

    std::unordered_map<AKTarget*, ShadowData> m_targets;
    ShadowData *m_currentData;
    SkIPoint m_offset;
    SkScalar m_radius;
    SkColor m_color;
    ShadowType m_type;
    std::bitset<FlagsLast> m_flags;
};

#endif // AKSHADOWEFFECT_H
