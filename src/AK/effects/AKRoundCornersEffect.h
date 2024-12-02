#ifndef AKROUNDCORNERSEFFECT_H
#define AKROUNDCORNERSEFFECT_H

#include <AK/AKObject.h>
#include <AK/AKBorderRadius.h>
#include <AK/AKBrush.h>
#include <include/core/SkPath.h>
#include <include/core/SkRegion.h>
#include <include/core/SkBlendMode.h>

class SkCanvas;

class AK::AKRoundCornersEffect : public AKObject
{
public:
    enum Corner
    {
        TL,
        TR,
        BL,
        BR,
        CornerLast
    };

    AKRoundCornersEffect(const AKBorderRadius &corners = AKBorderRadius()) noexcept :
        m_corners(corners)
    {
        m_brush.setBlendMode(SkBlendMode::kClear);
        m_brush.setAntiAlias(true);
    }

    void setCorners(const AKBorderRadius &corners) noexcept
    {
        m_corners = corners;
    }

    const AKBorderRadius &corners() const noexcept
    {
        return m_corners;
    }

    bool addDamage(const SkSize &nodeSize, const SkRegion *clip, SkRegion *damage) noexcept;
    bool clipCorners(SkCanvas *canvas, SkRegion *damage, SkRegion *opaque) noexcept;

private:
    AKBorderRadius m_corners;
    SkPath m_mask[4];
    SkIRect m_rect[4];
    AKBrush m_brush;
};

#endif // AKROUNDCORNERSEFFECT_H
