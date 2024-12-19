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

void AKBackgroundShadowEffect::onLayoutUpdate()
{
    if (shadowType() == Box)
        onLayoutUpdateBox();
}

void AKBackgroundShadowEffect::onLayoutUpdateBox() noexcept
{
    bool needsNewSurface { m_targets.find(currentTarget()) == m_targets.end() };
    bool needsFullDamage { needsNewSurface };
    const auto &chg { changes() };

    m_currentData = &m_targets[currentTarget()];

    if (m_currentData->prevScale != currentTarget()->xyScale())
    {
        needsNewSurface = needsFullDamage = true;
        m_currentData->prevScale = currentTarget()->xyScale();
    }

    effectRect = SkIRect::MakeWH(targetNode()->rect().width(), targetNode()->rect().height());
    effectRect.outset(shadowRadius(), shadowRadius());
    effectRect.offset(offset().x(), offset().y());

    const SkIPoint finalPos { effectRect.x() + targetNode()->rect().x(), effectRect.y() + targetNode()->rect().y() };

    /* TODO: border radius change */

    /* Shadow radius change */
    if (chg.test(Chg_ShadowType) || chg.test(Chg_ShadowRadius))
        needsFullDamage = needsNewSurface = true;

    const SkScalar centerSize { std::max(shadowRadius() * 0.5f, 64.f) };

    // TL
    m_currentData->dstRects[0] = SkIRect::MakeXYWH(
        finalPos.x(), finalPos.y(),
        shadowRadius(), shadowRadius());

    // T
    m_currentData->dstRects[1] = SkIRect::MakeXYWH(
        finalPos.x() + shadowRadius(), finalPos.y(),
        targetNode()->rect().width(), shadowRadius());

    // TR
    m_currentData->dstRects[2] = SkIRect::MakeXYWH(
        finalPos.x() + shadowRadius() + targetNode()->rect().width(), finalPos.y(),
        shadowRadius(), shadowRadius());

    // L
    m_currentData->dstRects[3] = SkIRect::MakeXYWH(
        finalPos.x(), finalPos.y() + shadowRadius(),
        shadowRadius(), targetNode()->rect().height());

    // Center
    m_currentData->dstRects[4] = SkIRect::MakeXYWH(
        finalPos.x() + shadowRadius(), finalPos.y() + shadowRadius(),
        targetNode()->rect().width(), targetNode()->rect().height());

    // R
    m_currentData->dstRects[5] = SkIRect::MakeXYWH(
        finalPos.x() + targetNode()->rect().width() + shadowRadius(), finalPos.y() + shadowRadius(),
        shadowRadius(), targetNode()->rect().height());

    // BL
    m_currentData->dstRects[6] = SkIRect::MakeXYWH(
        finalPos.x(), finalPos.y() + shadowRadius() + targetNode()->rect().height(),
        shadowRadius(), shadowRadius());

    // B
    m_currentData->dstRects[7] = SkIRect::MakeXYWH(
        finalPos.x() + shadowRadius(), finalPos.y() + shadowRadius() + targetNode()->rect().height(),
        targetNode()->rect().width(), shadowRadius());

    // BR
    m_currentData->dstRects[8] = SkIRect::MakeXYWH(
        finalPos.x() + shadowRadius() + targetNode()->rect().width(),
        finalPos.y() + shadowRadius() + targetNode()->rect().height(),
        shadowRadius(), shadowRadius());

    needsFullDamage |= chg.test(Chg_Color) || chg.test(Chg_ShadowClippingEnabled);

    if (needsNewSurface)
    {
        // TL
        m_currentData->srcRects[0] = SkRect::MakeWH(
            shadowRadius(),
            shadowRadius());

        // T
        m_currentData->srcRects[1] = SkRect::MakeXYWH(
            shadowRadius(), 0.f,
            centerSize, shadowRadius());

        // TR
        m_currentData->srcRects[2] = SkRect::MakeXYWH(
            shadowRadius() + centerSize, 0.f,
            shadowRadius(), shadowRadius());

        // L
        m_currentData->srcRects[3] = SkRect::MakeXYWH(
            0.f, shadowRadius(),
            shadowRadius(), centerSize);

        // Center
        m_currentData->srcRects[4] = SkRect::MakeXYWH(
            shadowRadius(), shadowRadius(),
            centerSize, centerSize);

        // R
        m_currentData->srcRects[5] = SkRect::MakeXYWH(
            shadowRadius() + centerSize, shadowRadius(),
            shadowRadius(), centerSize);

        // BL
        m_currentData->srcRects[6] = SkRect::MakeXYWH(
            0, shadowRadius() + centerSize,
            shadowRadius(), shadowRadius());

        // B
        m_currentData->srcRects[7] = SkRect::MakeXYWH(
            shadowRadius(), shadowRadius() + centerSize,
            centerSize, shadowRadius());

        // BR
        m_currentData->srcRects[8] = SkRect::MakeXYWH(
            centerSize + shadowRadius(),
            centerSize + shadowRadius(),
            shadowRadius(),
            shadowRadius());

        const SkSize surfaceSize { 2.f * shadowRadius() + centerSize, 2.f * shadowRadius() + centerSize };

        if (m_currentData->surface)
            m_currentData->surface->resize(surfaceSize, currentTarget()->xyScale());
        else
        {
            m_currentData->surface = AKSurface::Make(
                currentTarget()->surface->recordingContext(),
                surfaceSize,
                currentTarget()->xyScale(),
                true);
        }

        SkCanvas &canvas { *m_currentData->surface->surface()->getCanvas() };
        AKBrush brush;
        canvas.save();
        canvas.scale(
            currentTarget()->xyScale().x(),
            currentTarget()->xyScale().y());
        canvas.clear(SK_ColorTRANSPARENT);
        brush.setBlendMode(SkBlendMode::kSrc);
        brush.setColor(SK_ColorWHITE);
        brush.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, shadowRadius()/3.f));
        canvas.drawRect(SkRect::MakeXYWH(shadowRadius(), shadowRadius(), centerSize, centerSize), brush);
        canvas.restore();
    }

    if (needsFullDamage)
        addDamage(AK_IRECT_INF);
}

void AKBackgroundShadowEffect::onRender(AKPainter *painter, const SkRegion &damage)
{
    if (shadowType() == Box)
        onRenderBox(painter, damage);
}

void AKBackgroundShadowEffect::onRenderBox(AKPainter *painter, const SkRegion &damage) noexcept
{
    SkRegion fullDamage { damage };

    if (shadowClippingEnabled())
        fullDamage.op(targetNode()->rect(), SkRegion::Op::kDifference_Op);

    for (int i = 0; i < 9; i++)
    {
        SkRegion clippedDamage { fullDamage };
        clippedDamage.op(m_currentData->dstRects[i], SkRegion::Op::kIntersect_Op);

        painter->bindTextureMode({
            .texture = m_currentData->surface->image(),
            .pos = { m_currentData->dstRects[i].x(), m_currentData->dstRects[i].y() },
            .srcRect = m_currentData->srcRects[i],
            .dstSize = m_currentData->dstRects[i].size(),
            .srcTransform = AKTransform::Normal,
            .srcScale = m_currentData->surface->scale().x()
        });

        painter->drawRegion(clippedDamage);
    }
}

void AKBackgroundShadowEffect::onTargetNodeChanged()
{

}
