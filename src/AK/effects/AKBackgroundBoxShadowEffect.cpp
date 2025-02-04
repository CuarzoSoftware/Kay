#include <include/core/SkCanvas.h>
#include <include/effects/SkBlurMaskFilter.h>
#include <include/effects/SkColorMatrixFilter.h>
#include <include/core/SkRRect.h>
#include <AK/effects/AKBackgroundBoxShadowEffect.h>
#include <AK/AKTarget.h>
#include <AK/AKSurface.h>
#include <AK/AKBrush.h>

using namespace AK;

void AKBackgroundBoxShadowEffect::onSceneCalculatedRect()
{
    if (m_borderRadius.fBL != 0 || m_borderRadius.fBR != 0 || m_borderRadius.fTL != 0 || m_borderRadius.fTR != 0)
    {
        onSceneCalculatedRectWithBorderRadius();
        return;
    }

    bool needsNewSurface { m_targets.find(currentTarget()) == m_targets.end() };
    bool needsFullDamage { needsNewSurface };
    const auto &chg { changes() };

    m_currentData = &m_targets[currentTarget()];

    if (!m_currentData->surface)
    {
        currentTarget()->AKObject::on.destroyed.subscribe(this, [this](AKObject *obj){
            AKTarget *target { static_cast<AKTarget*>(obj) };
            m_targets.erase(target);
        });
    }

    if (m_currentData->prevScale != currentTarget()->bakedComponentsScale())
    {
        needsNewSurface = needsFullDamage = true;
        m_currentData->prevScale = currentTarget()->bakedComponentsScale();
    }

    effectRect = SkIRect::MakeWH(targetNode()->rect().width(), targetNode()->rect().height());
    effectRect.outset(m_radius, m_radius);
    effectRect.offset(offset().x(), offset().y());

    const SkIPoint finalPos { effectRect.x() + targetNode()->rect().x(), effectRect.y() + targetNode()->rect().y() };

    if (chg.test(Chg_Radius))
        needsFullDamage = needsNewSurface = true;

    const SkScalar centerSize { std::max(m_radius * 0.5f, 64.f) };

    // TL
    m_currentData->dstRects[0] = SkIRect::MakeXYWH(
        finalPos.x(), finalPos.y(),
        m_radius, m_radius);

    // T
    m_currentData->dstRects[1] = SkIRect::MakeXYWH(
        finalPos.x() + m_radius, finalPos.y(),
        targetNode()->rect().width(), m_radius);

    // TR
    m_currentData->dstRects[2] = SkIRect::MakeXYWH(
        finalPos.x() + m_radius + targetNode()->rect().width(), finalPos.y(),
        m_radius, m_radius);

    // L
    m_currentData->dstRects[3] = SkIRect::MakeXYWH(
        finalPos.x(), finalPos.y() + m_radius,
        m_radius, targetNode()->rect().height());

    // Center
    m_currentData->dstRects[4] = SkIRect::MakeXYWH(
        finalPos.x() + m_radius, finalPos.y() + m_radius,
        targetNode()->rect().width(), targetNode()->rect().height());

    // R
    m_currentData->dstRects[5] = SkIRect::MakeXYWH(
        finalPos.x() + targetNode()->rect().width() + m_radius, finalPos.y() + m_radius,
        m_radius, targetNode()->rect().height());

    // BL
    m_currentData->dstRects[6] = SkIRect::MakeXYWH(
        finalPos.x(), finalPos.y() + m_radius + targetNode()->rect().height(),
        m_radius, m_radius);

    // B
    m_currentData->dstRects[7] = SkIRect::MakeXYWH(
        finalPos.x() + m_radius, finalPos.y() + m_radius + targetNode()->rect().height(),
        targetNode()->rect().width(), m_radius);

    // BR
    m_currentData->dstRects[8] = SkIRect::MakeXYWH(
        finalPos.x() + m_radius + targetNode()->rect().width(),
        finalPos.y() + m_radius + targetNode()->rect().height(),
        m_radius, m_radius);

    needsFullDamage |= chg.test(Chg_Color) || chg.test(Chg_FillBackground);

    if (needsNewSurface)
    {
        // TL
        m_currentData->srcRects[0] = SkRect::MakeWH(
            m_radius,
            m_radius);

        // T
        m_currentData->srcRects[1] = SkRect::MakeXYWH(
            m_radius, 0.f,
            centerSize, m_radius);

        // TR
        m_currentData->srcRects[2] = SkRect::MakeXYWH(
            m_radius + centerSize, 0.f,
            m_radius, m_radius);

        // L
        m_currentData->srcRects[3] = SkRect::MakeXYWH(
            0.f, m_radius,
            m_radius, centerSize);

        // Center
        m_currentData->srcRects[4] = SkRect::MakeXYWH(
            m_radius, m_radius,
            centerSize, centerSize);

        // R
        m_currentData->srcRects[5] = SkRect::MakeXYWH(
            m_radius + centerSize, m_radius,
            m_radius, centerSize);

        // BL
        m_currentData->srcRects[6] = SkRect::MakeXYWH(
            0, m_radius + centerSize,
            m_radius, m_radius);

        // B
        m_currentData->srcRects[7] = SkRect::MakeXYWH(
            m_radius, m_radius + centerSize,
            centerSize, m_radius);

        // BR
        m_currentData->srcRects[8] = SkRect::MakeXYWH(
            centerSize + m_radius,
            centerSize + m_radius,
            m_radius,
            m_radius);

        const SkSize surfaceSize { 2.f * m_radius + centerSize, 2.f * m_radius + centerSize };

        if (m_currentData->surface)
            m_currentData->surface->resize(surfaceSize, currentTarget()->bakedComponentsScale());
        else
        {
            m_currentData->surface = AKSurface::Make(
                surfaceSize,
                currentTarget()->bakedComponentsScale(),
                true);
        }

        SkCanvas &canvas { *m_currentData->surface->surface()->getCanvas() };
        AKBrush brush;
        canvas.save();
        canvas.scale(
            m_currentData->surface->scale(),
            m_currentData->surface->scale());
        canvas.clear(SK_ColorTRANSPARENT);
        brush.setBlendMode(SkBlendMode::kSrc);
        brush.setColor(SK_ColorWHITE);
        brush.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, m_radius/3.f));
        canvas.drawRect(SkRect::MakeXYWH(m_radius, m_radius, centerSize, centerSize), brush);
        canvas.restore();
    }

    if (needsFullDamage)
        addDamage(AK_IRECT_INF);
}

// TODO: Use nine patch if fill background is enabled
void AKBackgroundBoxShadowEffect::onSceneCalculatedRectWithBorderRadius() noexcept
{
    bool needsNewSurface { m_targets.find(currentTarget()) == m_targets.end() };
    bool needsFullDamage { needsNewSurface };
    const auto &chg { changes() };

    m_currentData = &m_targets[currentTarget()];

    if (!m_currentData->surface)
    {
        currentTarget()->AKObject::on.destroyed.subscribe(this, [this](AKObject *obj){
            AKTarget *target { static_cast<AKTarget*>(obj) };
            m_targets.erase(target);
        });
    }

    if (m_currentData->prevScale != currentTarget()->bakedComponentsScale())
    {
        needsNewSurface = needsFullDamage = true;
        m_currentData->prevScale = currentTarget()->bakedComponentsScale();
    }

    const SkISize prevSize { effectRect.size() };

    effectRect = SkIRect::MakeWH(targetNode()->rect().width(), targetNode()->rect().height());
    effectRect.outset(m_radius, m_radius);
    effectRect.offset(offset().x(), offset().y());

    if (effectRect.size() != prevSize || chg.test(Chg_Radius) || chg.test(Chg_BorderRadius) || chg.test(Chg_Offset) || chg.test(Chg_FillBackground))
        needsFullDamage = needsNewSurface = true;

    if (needsNewSurface)
    {
        const SkSize surfaceSize { SkSize::Make(effectRect.size()) };

        if (m_currentData->surface)
            m_currentData->surface->resize(surfaceSize, currentTarget()->bakedComponentsScale());
        else
        {
            m_currentData->surface = AKSurface::Make(
                surfaceSize,
                currentTarget()->bakedComponentsScale(),
                true);
        }

        SkCanvas &canvas { *m_currentData->surface->surface()->getCanvas() };
        AKBrush brush;
        canvas.save();
        canvas.scale(
            m_currentData->surface->scale(),
            m_currentData->surface->scale());
        canvas.clipIRect(SkIRect::MakeWH(effectRect.size().width() + 1, effectRect.size().height() + 1));
        canvas.clear(SK_ColorTRANSPARENT);
        brush.setBlendMode(SkBlendMode::kSrc);
        brush.setColor(SK_ColorWHITE);
        brush.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, m_radius/3.f));

        SkVector radii[4];
        radii[SkRRect::kUpperLeft_Corner].set(m_borderRadius.fTL, m_borderRadius.fTL);
        radii[SkRRect::kUpperRight_Corner].set(m_borderRadius.fTR, m_borderRadius.fTR);
        radii[SkRRect::kLowerRight_Corner].set(m_borderRadius.fBR, m_borderRadius.fBR);
        radii[SkRRect::kLowerLeft_Corner].set(m_borderRadius.fBL, m_borderRadius.fBL);

        SkRRect rrect;
        rrect.setRectRadii(
            SkRect::MakeXYWH(m_radius, m_radius, targetNode()->rect().width(), targetNode()->rect().height()),
            radii);

        canvas.drawRRect(rrect, brush);

        if (!fillBackgroundEnabled())
        {
            rrect.setRectRadii(
                SkRect::MakeXYWH(m_radius - offset().x() + 0.5f,
                                 m_radius - offset().y() + 0.5f,
                                 targetNode()->rect().width() - 1.f,
                                 targetNode()->rect().height() - 1.f),
                radii);
            brush.setAntiAlias(true);
            brush.setBlendMode(SkBlendMode::kClear);
            brush.setMaskFilter(nullptr);
            canvas.drawRRect(rrect, brush);
        }

        canvas.restore();
    }

    if (needsFullDamage)
        addDamage(AK_IRECT_INF);
}


void AKBackgroundBoxShadowEffect::onRender(AKPainter *painter, const SkRegion &damage)
{
    if (m_borderRadius.fBL != 0 || m_borderRadius.fBR != 0 || m_borderRadius.fTL != 0 || m_borderRadius.fTR != 0)
    {
        onRenderWithBorderRadius(painter, damage);
        return;
    }

    SkRegion fullDamage { damage };

    if (!fillBackgroundEnabled())
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
            .srcScale = SkScalar(m_currentData->surface->scale())
        });

        painter->drawRegion(clippedDamage);
    }
}

void AKBackgroundBoxShadowEffect::onRenderWithBorderRadius(AKPainter *painter, const SkRegion &damage) noexcept
{
    SkRegion finalDamage { damage };

    if (!fillBackgroundEnabled())
    {
        // Eliminate damaged regions without round corners
        // to optimize GPU usage by preventing unnecessary rendering of invisible pixels
        SkRegion toExclude;
        toExclude.setRect(targetNode()->rect());
        toExclude.op(SkIRect::MakeXYWH( // TL
            targetNode()->rect().x(),
            targetNode()->rect().y(),
            m_borderRadius.fTL,
            m_borderRadius.fTL), SkRegion::Op::kDifference_Op);
        toExclude.op(SkIRect::MakeXYWH( // TR
                         targetNode()->rect().fRight - m_borderRadius.fTR,
                         targetNode()->rect().y(),
                         m_borderRadius.fTR,
                         m_borderRadius.fTR), SkRegion::Op::kDifference_Op);
        toExclude.op(SkIRect::MakeXYWH( // BR
                         targetNode()->rect().fRight - m_borderRadius.fBR,
                         targetNode()->rect().fBottom - m_borderRadius.fBR,
                         m_borderRadius.fBR,
                         m_borderRadius.fBR), SkRegion::Op::kDifference_Op);
        toExclude.op(SkIRect::MakeXYWH( // BL
                         targetNode()->rect().x(),
                         targetNode()->rect().fBottom - m_borderRadius.fBL,
                         m_borderRadius.fBL,
                         m_borderRadius.fBL), SkRegion::Op::kDifference_Op);
        finalDamage.op(toExclude, SkRegion::Op::kDifference_Op);
    }

    const SkRect src { SkRect::MakeWH(rect().size().width(), rect().size().height()) };

    painter->bindTextureMode({
        .texture = m_currentData->surface->image(),
        .pos = { rect().x(), rect().y() },
        .srcRect = src,
        .dstSize = rect().size(),
        .srcTransform = AKTransform::Normal,
        .srcScale = SkScalar(m_currentData->surface->scale())
    });

    painter->drawRegion(finalDamage);
}

void AKBackgroundBoxShadowEffect::onTargetNodeChanged()
{

}
