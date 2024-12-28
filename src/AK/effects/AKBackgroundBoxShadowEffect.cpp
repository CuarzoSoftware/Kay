#include <include/core/SkCanvas.h>
#include <include/effects/SkBlurMaskFilter.h>
#include <include/effects/SkColorMatrixFilter.h>
#include <AK/effects/AKBackgroundBoxShadowEffect.h>
#include <AK/AKTarget.h>
#include <AK/AKSurface.h>
#include <AK/AKBrush.h>

using namespace AK;

void AKBackgroundBoxShadowEffect::onLayoutUpdate()
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

    if (m_currentData->prevScale != currentTarget()->xyScale())
    {
        needsNewSurface = needsFullDamage = true;
        m_currentData->prevScale = currentTarget()->xyScale();
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
            m_currentData->surface->resize(surfaceSize, currentTarget()->xyScale());
        else
        {
            m_currentData->surface = AKSurface::Make(
                currentTarget()->surface()->recordingContext(),
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
        brush.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, m_radius/3.f));
        canvas.drawRect(SkRect::MakeXYWH(m_radius, m_radius, centerSize, centerSize), brush);
        canvas.restore();
    }

    if (needsFullDamage)
        addDamage(AK_IRECT_INF);
}


void AKBackgroundBoxShadowEffect::onRender(AKPainter *painter, const SkRegion &damage)
{
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
            .srcScale = m_currentData->surface->scale().x()
        });

        painter->drawRegion(clippedDamage);
    }
}

void AKBackgroundBoxShadowEffect::onTargetNodeChanged()
{

}
