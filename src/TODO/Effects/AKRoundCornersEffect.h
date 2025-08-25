#ifndef CZ_AKROUNDCORNERSEFFECT_H
#define CZ_AKROUNDCORNERSEFFECT_H

#include <CZ/AK/AKObject.h>
#include <CZ/AK/AKBorderRadius.h>
#include <CZ/skia/core/SkPaint.h>
#include <CZ/skia/core/SkPath.h>
#include <CZ/skia/core/SkRegion.h>
#include <CZ/skia/core/SkBlendMode.h>

class SkCanvas;

class CZ::AKRoundCornersEffect : public AKObject
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

    CZ_DISABLE_COPY(AKRoundCornersEffect)

    void setCorners(const AKBorderRadius &corners) noexcept
    {
        m_corners = corners;
    }

    const AKBorderRadius &corners() const noexcept
    {
        return m_corners;
    }

    void setPen(const SkPaint &pen) noexcept
    {
        m_pen = pen;
    }

    bool addDamage(const SkISize &nodeSize, const SkRegion *clip, SkRegion *damage) noexcept;
    bool clipCorners(SkCanvas *canvas, const SkRegion *clip, SkRegion *damage, SkRegion *opaque) noexcept;

private:
    AKBorderRadius m_corners;
    SkPath m_mask[4];
    SkIRect m_rect[4];
    SkPaint m_brush;
    SkPaint m_pen {};
};

#endif // CZ_AKROUNDCORNERSEFFECT_H
