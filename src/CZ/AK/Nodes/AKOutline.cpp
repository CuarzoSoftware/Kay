#include <CZ/AK/Nodes/AKOutline.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RPass.h>
#include <CZ/skia/core/SkCanvas.h>
#include <CZ/skia/core/SkRRect.h>
#include <algorithm>

using namespace CZ;

AKOutline::AKOutline(AKNode *parent) noexcept : AKNinePatch(nullptr, {}, parent)
{
    // Nothing inside the ring: never painted (part of the invisible region)
    setRegionVisibility(C, false);

    // Purely decorative
    const SkRegion empty;
    setInputRegion(&empty);
    addChange(CHOutline);
}

void AKOutline::setColor(CZAdaptiveColor color) noexcept
{
    if (m_color == color)
        return;

    m_color = color;
    addChange(CHOutline);
}

void AKOutline::setRadius(Int32 radius) noexcept
{
    radius = std::max(0, radius);

    if (m_radius == radius)
        return;

    m_radius = radius;
    addChange(CHOutline);
}

void AKOutline::setThickness(Int32 thickness) noexcept
{
    thickness = std::max(1, thickness);

    if (m_thickness == thickness)
        return;

    m_thickness = thickness;
    addChange(CHOutline);
}

void AKOutline::onSceneBegin()
{
    if (changes().testAnyOf(CHOutline, CHLayoutScale, CHColorScheme))
    {
        // The ring's corners around a single stretchable pixel (the center)
        const Int32 outer { m_radius + m_thickness };
        const Int32 size { 2 * outer + 1 };
        const Int32 scale { std::max(1, Int32(this->scale())) };

        if (auto surface { RSurface::Make(SkISize(size, size), scale, true) })
        {
            if (auto pass { surface->beginPass(RPassCap_SkCanvas) })
            {
                auto &c { *pass->getCanvas() };
                c.clear(SK_ColorTRANSPARENT);

                SkPaint paint;
                paint.setAntiAlias(true);
                paint.setColor(m_color.resolve(colorScheme()));

                const SkRRect out { SkRRect::MakeRectXY(SkRect::MakeWH(size, size), outer, outer) };
                const SkRect innerRect { SkRect::MakeLTRB(m_thickness, m_thickness, size - m_thickness, size - m_thickness) };
                const SkRRect in { SkRRect::MakeRectXY(innerRect, m_radius, m_radius) };
                c.drawDRRect(out, in, paint);
            }

            setImage(surface->image());
            setCenter(SkIRect::MakeXYWH(outer, outer, 1, 1));
        }
    }

    AKNinePatch::onSceneBegin();
}
