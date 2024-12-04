#include <AK/AKTarget.h>
#include <include/core/SkCanvas.h>
#include <include/effects/SkBlurMaskFilter.h>
#include <include/effects/SkColorMatrixFilter.h>
#include <AK/effects/AKBackgroundShadowEffect.h>
#include <AK/AKSurface.h>
#include <AK/AKBrush.h>
#include <AK/nodes/AKRoundContainer.h>

/*
 * Case 1: Box without border radius but with or without offset.
 *
 * Bake image nine with a center size = radius / 2, then draw clipped onRender().
 *
 *
 */

using namespace AK;

void AKBackgroundShadowEffect::onSceneBegin()
{
    if (type() == Box)
        onSceneBeginBox();
}

void AKBackgroundShadowEffect::onSceneBeginBox() noexcept
{
    opaqueRegion.setEmpty();

    bool needsNewSurface { m_targets.find(currentTarget()) == m_targets.end() };
    bool needsFullDamage { needsNewSurface };
    m_currentData = &m_targets[currentTarget()];

    /* TODO: type change */

    /* TODO: border radius change */

    /* Shadow radius change */
    if (radius() != m_currentData->radius)
    {
        needsFullDamage = needsNewSurface = true;
        m_currentData->radius = radius();
    }

    /* Color change */
    if (!m_currentData->brush.getColorFilter() || color() != m_currentData->brush.getColor())
    {
        needsFullDamage = true;
        m_currentData->brush.setColor(color());
        m_currentData->brush.setColorFilter(SkColorFilters::Blend(color(), SkBlendMode::kSrcIn));
    }

    /* Clip change */
    if (m_currentData->flags.test(Clipping) != clippingEnabled())
    {
        needsFullDamage = true;
        m_currentData->flags.set(Clipping, clippingEnabled());
    }

    rect = SkIRect::MakeWH(targetNode()->rect().width(), targetNode()->rect().height());
    rect.outset(radius(), radius());
    rect.offset(offset().x(), offset().y());

    if (needsNewSurface)
    {
        const SkScalar nineCenterSize { std::max(radius() * 0.5f, 64.f) };
        const SkScalar surfaceSize { (radius() * 2.f) + nineCenterSize };

        m_currentData->shadowSurface = AKSurface::Make(
            currentTarget()->surface->recordingContext(),
            SkSize::Make(surfaceSize, surfaceSize),
            currentTarget()->xyScale(),
            true);

        m_currentData->nineCenter = SkIRect::MakeXYWH(
            radius() * currentTarget()->xyScale().x(),
            radius() * currentTarget()->xyScale().y(),
            nineCenterSize * currentTarget()->xyScale().x(),
            nineCenterSize * currentTarget()->xyScale().y());

        SkCanvas &canvas { *m_currentData->shadowSurface->surface()->getCanvas() };
        canvas.save();
        canvas.clear(SK_ColorTRANSPARENT);
        canvas.scale(
            currentTarget()->xyScale().x(),
            currentTarget()->xyScale().y());

        AKBrush brush;
        brush.setColor(SK_ColorWHITE);
        brush.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, radius()/3.f));
        canvas.drawRect(SkRect::MakeXYWH(radius(), radius(), nineCenterSize, nineCenterSize), brush);
        canvas.restore();
    }

    if (needsFullDamage)
        addDamage(AK_IRECT_INF);
}

void AKBackgroundShadowEffect::onRender(SkCanvas *canvas, const SkRegion &damage, bool /*opaque*/)
{
    if (type() == Box)
        onRenderBox(canvas, damage);
}

void AKBackgroundShadowEffect::onRenderBox(SkCanvas *canvas, const SkRegion &damage) noexcept
{
    const SkVector xyInvScale {
        1.f/m_currentData->shadowSurface->scale().x(),
        1.f/m_currentData->shadowSurface->scale().y() };

    SkRegion finalDamage { damage };

    if (m_currentData->flags.test(Clipping))
    {
        finalDamage.op(SkIRect::MakeXYWH(
            targetNode()->rect().x() - AKNode::rect().x(),
            targetNode()->rect().y() - AKNode::rect().y(),
            targetNode()->rect().width(),
            targetNode()->rect().height()),
            SkRegion::Op::kDifference_Op);
    }

    SkRegion::Iterator it(finalDamage);
    while (!it.done())
    {
        canvas->save();
        canvas->clipIRect(it.rect());
        canvas->scale(xyInvScale.x(), xyInvScale.y());
        canvas->drawImageNine(
            m_currentData->shadowSurface->image().get(),
            m_currentData->nineCenter,
            SkRect::MakeWH(
                rect.width() * m_currentData->shadowSurface->scale().x(),
                rect.height()* m_currentData->shadowSurface->scale().y()),
            SkFilterMode::kLinear, &m_currentData->brush);
        canvas->restore();
        it.next();
    }
}

void AKBackgroundShadowEffect::onTargetNodeChanged()
{

}
