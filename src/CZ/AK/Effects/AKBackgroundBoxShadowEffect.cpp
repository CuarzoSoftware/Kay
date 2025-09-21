#include <CZ/skia/core/SkCanvas.h>
#include <CZ/skia/effects/SkBlurMaskFilter.h>
#include <CZ/skia/effects/SkColorMatrixFilter.h>
#include <CZ/skia/core/SkRRect.h>
#include <CZ/AK/Events/AKRenderEvent.h>
#include <CZ/AK/Effects/AKBackgroundBoxShadowEffect.h>
#include <CZ/AK/AKTarget.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RPass.h>
#include <CZ/skia/core/SkPaint.h>

using namespace CZ;

void AKBackgroundBoxShadowEffect::onSceneCalculatedRect()
{
    const auto &chg { changes() };

    if (m_prevRect == targetNode()->worldRect() &&
        m_prevScale == targetNode()->scale() &&
        !chg.testAnyOf(CHOffset, CHFillBackground, CHBorderRadius, CHRadius) &&
        m_surface)
        return;

    if (m_borderRadius.fBL != 0 || m_borderRadius.fBR != 0 || m_borderRadius.fTL != 0 || m_borderRadius.fTR != 0)
    {
        onSceneCalculatedRectWithBorderRadius();
        return;
    }

    bool needsNewSurface { m_surface == nullptr };
    bool needsFullDamage { needsNewSurface };

    if (m_prevScale != targetNode()->scale())
    {
        needsNewSurface = needsFullDamage = true;
        m_prevScale = targetNode()->scale();
    }

    effectRect = SkIRect::MakeWH(targetNode()->worldRect().width(), targetNode()->worldRect().height());
    effectRect.outset(m_radius, m_radius);
    effectRect.offset(offset().x(), offset().y());

    const SkIPoint finalPos { effectRect.x() + targetNode()->sceneRect().x(), effectRect.y() + targetNode()->sceneRect().y() };

    if (chg.test(CHRadius))
        needsFullDamage = needsNewSurface = true;

    const SkScalar centerSize { std::max(m_radius * 0.5f, 64.f) };

    // TL
    m_dstRects[0] = SkIRect::MakeXYWH(
        finalPos.x(), finalPos.y(),
        m_radius, m_radius);

    // T
    m_dstRects[1] = SkIRect::MakeXYWH(
        finalPos.x() + m_radius, finalPos.y(),
        targetNode()->sceneRect().width(), m_radius);

    // TR
    m_dstRects[2] = SkIRect::MakeXYWH(
        finalPos.x() + m_radius + targetNode()->sceneRect().width(), finalPos.y(),
        m_radius, m_radius);

    // L
    m_dstRects[3] = SkIRect::MakeXYWH(
        finalPos.x(), finalPos.y() + m_radius,
        m_radius, targetNode()->sceneRect().height());

    // Center
    m_dstRects[4] = SkIRect::MakeXYWH(
        finalPos.x() + m_radius, finalPos.y() + m_radius,
        targetNode()->sceneRect().width(), targetNode()->sceneRect().height());

    // R
    m_dstRects[5] = SkIRect::MakeXYWH(
        finalPos.x() + targetNode()->sceneRect().width() + m_radius, finalPos.y() + m_radius,
        m_radius, targetNode()->sceneRect().height());

    // BL
    m_dstRects[6] = SkIRect::MakeXYWH(
        finalPos.x(), finalPos.y() + m_radius + targetNode()->sceneRect().height(),
        m_radius, m_radius);

    // B
    m_dstRects[7] = SkIRect::MakeXYWH(
        finalPos.x() + m_radius, finalPos.y() + m_radius + targetNode()->sceneRect().height(),
        targetNode()->sceneRect().width(), m_radius);

    // BR
    m_dstRects[8] = SkIRect::MakeXYWH(
        finalPos.x() + m_radius + targetNode()->sceneRect().width(),
        finalPos.y() + m_radius + targetNode()->sceneRect().height(),
        m_radius, m_radius);

    needsFullDamage |= chg.testAnyOf(CHColor, CHFillBackground);

    if (needsNewSurface)
    {
        // TL
        m_srcRects[0] = SkRect::MakeWH(
            m_radius,
            m_radius);

        // T
        m_srcRects[1] = SkRect::MakeXYWH(
            m_radius, 0.f,
            centerSize, m_radius);

        // TR
        m_srcRects[2] = SkRect::MakeXYWH(
            m_radius + centerSize, 0.f,
            m_radius, m_radius);

        // L
        m_srcRects[3] = SkRect::MakeXYWH(
            0.f, m_radius,
            m_radius, centerSize);

        // Center
        m_srcRects[4] = SkRect::MakeXYWH(
            m_radius, m_radius,
            centerSize, centerSize);

        // R
        m_srcRects[5] = SkRect::MakeXYWH(
            m_radius + centerSize, m_radius,
            m_radius, centerSize);

        // BL
        m_srcRects[6] = SkRect::MakeXYWH(
            0, m_radius + centerSize,
            m_radius, m_radius);

        // B
        m_srcRects[7] = SkRect::MakeXYWH(
            m_radius, m_radius + centerSize,
            centerSize, m_radius);

        // BR
        m_srcRects[8] = SkRect::MakeXYWH(
            centerSize + m_radius,
            centerSize + m_radius,
            m_radius,
            m_radius);

        const SkISize surfaceSize (2.f * m_radius + centerSize, 2.f * m_radius + centerSize);

        if (m_surface)
            m_surface->resize(surfaceSize, targetNode()->scale());
        else
        {
            m_surface = RSurface::Make(
                surfaceSize,
                targetNode()->scale(),
                true);
        }

        auto pass { m_surface->beginPass() };
        SkCanvas &canvas { *pass->getCanvas() };
        SkPaint brush;
        canvas.save();
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
    bool needsNewSurface { m_surface == nullptr };
    bool needsFullDamage { needsNewSurface };
    const auto &chg { changes() };

    if (m_prevScale != targetNode()->scale())
    {
        needsNewSurface = needsFullDamage = true;
        m_prevScale = targetNode()->scale();
    }

    const SkISize prevSize { effectRect.size() };

    effectRect = SkIRect::MakeWH(targetNode()->sceneRect().width(), targetNode()->sceneRect().height());
    effectRect.outset(m_radius, m_radius);
    effectRect.offset(offset().x(), offset().y());

    if (effectRect.size() != prevSize || chg.testAnyOf(CHRadius, CHBorderRadius, CHOffset, CHFillBackground))
        needsFullDamage = needsNewSurface = true;

    if (needsNewSurface)
    {
        const SkISize surfaceSize { effectRect.size() };

        if (m_surface)
            m_surface->resize(surfaceSize, targetNode()->scale());
        else
        {
            m_surface = RSurface::Make(
                surfaceSize,
                targetNode()->scale(),
                true);
        }

        auto pass { m_surface->beginPass() };
        SkCanvas &canvas { *pass->getCanvas() };
        SkPaint brush;
        canvas.save();
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
            SkRect::MakeXYWH(m_radius, m_radius, targetNode()->sceneRect().width(), targetNode()->sceneRect().height()),
            radii);

        canvas.drawRRect(rrect, brush);

        if (!fillBackgroundEnabled())
        {
            rrect.setRectRadii(
                SkRect::MakeXYWH(m_radius - offset().x() + 0.5f,
                                 m_radius - offset().y() + 0.5f,
                                 targetNode()->sceneRect().width() - 1.f,
                                 targetNode()->sceneRect().height() - 1.f),
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


void AKBackgroundBoxShadowEffect::renderEvent(const AKRenderEvent &p)
{
    if (m_borderRadius.fBL != 0 || m_borderRadius.fBR != 0 || m_borderRadius.fTL != 0 || m_borderRadius.fTR != 0)
    {
        onRenderWithBorderRadius(p);
        return;
    }

    SkRegion fullDamage { p.damage };

    if (!fillBackgroundEnabled())
        fullDamage.op(targetNode()->sceneRect(), SkRegion::Op::kDifference_Op);

    auto *painter { p.pass->getPainter() };
    RDrawImageInfo info {};

    for (int i = 0; i < 9; i++)
    {
        info.image = m_surface->image();
        info.dst = m_dstRects[i];
        info.src = m_srcRects[i];
        info.srcScale = targetNode()->scale();
        painter->drawImage(info, &fullDamage);
    }
}

void AKBackgroundBoxShadowEffect::onRenderWithBorderRadius(const AKRenderEvent &p) noexcept
{
    SkRegion finalDamage { p.damage };

    if (!fillBackgroundEnabled())
    {
        // Eliminate damaged regions without round corners
        // to optimize GPU usage by preventing unnecessary rendering of invisible pixels
        SkRegion toExclude;
        toExclude.setRect(targetNode()->sceneRect());
        toExclude.op(SkIRect::MakeXYWH( // TL
            targetNode()->sceneRect().x(),
            targetNode()->sceneRect().y(),
            m_borderRadius.fTL,
            m_borderRadius.fTL), SkRegion::Op::kDifference_Op);
        toExclude.op(SkIRect::MakeXYWH( // TR
                         targetNode()->sceneRect().fRight - m_borderRadius.fTR,
                         targetNode()->sceneRect().y(),
                         m_borderRadius.fTR,
                         m_borderRadius.fTR), SkRegion::Op::kDifference_Op);
        toExclude.op(SkIRect::MakeXYWH( // BR
                         targetNode()->sceneRect().fRight - m_borderRadius.fBR,
                         targetNode()->sceneRect().fBottom - m_borderRadius.fBR,
                         m_borderRadius.fBR,
                         m_borderRadius.fBR), SkRegion::Op::kDifference_Op);
        toExclude.op(SkIRect::MakeXYWH( // BL
                         targetNode()->sceneRect().x(),
                         targetNode()->sceneRect().fBottom - m_borderRadius.fBL,
                         m_borderRadius.fBL,
                         m_borderRadius.fBL), SkRegion::Op::kDifference_Op);
        finalDamage.op(toExclude, SkRegion::Op::kDifference_Op);
    }

    auto *painter { p.pass->getPainter() };
    RDrawImageInfo info {};
    info.image = m_surface->image();
    info.src = SkRect::MakeWH(sceneRect().size().width(), sceneRect().size().height());
    info.dst = sceneRect();
    info.srcScale = targetNode()->scale();
    painter->drawImage(info, &finalDamage);
}

void AKBackgroundBoxShadowEffect::onTargetNodeChanged() {}
