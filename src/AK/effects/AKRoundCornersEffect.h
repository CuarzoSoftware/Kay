#ifndef AKROUNDCORNERSEFFECT_H
#define AKROUNDCORNERSEFFECT_H

#include <AK/AKObject.h>
#include <AK/AKBorderRadius.h>
#include <AK/AKBrush.h>
#include <AK/AKPen.h>
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

    AKCLASS_NO_COPY(AKRoundCornersEffect)

    void setCorners(const AKBorderRadius &corners) noexcept
    {
        m_corners = corners;
    }

    const AKBorderRadius &corners() const noexcept
    {
        return m_corners;
    }

    void setPen(const AKPen &pen) noexcept
    {
        m_pen = pen;
    }

    bool addDamage(const SkISize &nodeSize, const SkRegion *clip, SkRegion *damage) noexcept;
    bool clipCorners(SkCanvas *canvas, const SkRegion *clip, SkRegion *damage, SkRegion *opaque) noexcept;

private:
    AKBorderRadius m_corners;
    SkPath m_mask[4];
    SkIRect m_rect[4];
    AKBrush m_brush;
    AKPen m_pen { AKPen::NoPen() };
};

#endif // AKROUNDCORNERSEFFECT_H
